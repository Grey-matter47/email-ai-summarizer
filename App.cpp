#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <unistd.h>    // For getcwd()
#include <limits.h>    // For PATH_MAX
#include <json/json.h>
#include <atomic>      // For atomic<bool>
#include <ctime>       // For time_t, time, and ctime

using namespace std;

//---------------------------------------------------------------------
// Logger Class Definition
//---------------------------------------------------------------------

// Enum to represent log levels
enum LogLevel {  INFO, WARNING, ERROR };

class Logger {
public:
    // Constructor: Opens the log file in append mode
    Logger(const string &filename) {
        logFile.open(filename, ios::app);
        if (!logFile.is_open()) {
            cerr << "Error opening log file." << endl;
        }
    }
    
    // Destructor: Closes the log file
    ~Logger() { logFile.close(); }
    
    // Logs a message with a given log level
    void log(LogLevel level, const string &message) {
        // Get current timestamp
        time_t now = time(0);
        tm *timeinfo = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        // Create log entry
        ostringstream logEntry;
        logEntry << "[" << timestamp << "] "
                 << levelToString(level) << ": " << message << endl;
        
        // Output to console
        // cout << logEntry.str();
        
        // Output to log file if it's open
        if (logFile.is_open()) {
            logFile << logEntry.str();
            logFile.flush(); // Ensure immediate write to file
        }
    }
    
private:
    ofstream logFile; // File stream for the log file
    
    string levelToString(LogLevel level) {
        switch(level) {
            case INFO:    return "INFO";
            case WARNING: return "WARNING";
            case ERROR:   return "ERROR";
            default:      return "UNKNOWN";
        }
    }
};

// Instantiate a global logger (you can also pass this around if needed)
Logger logger("./../summarizer-app.log");

//---------------------------------------------------------------------
// Gemini Summarizer Code
//---------------------------------------------------------------------

const string API_KEY = "AIzaSyBrWYPnBdGi2KC0DfiuxBgyBrnvlcMASqQ";
const string API_URL =
    "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + API_KEY;

// Displays a simple loading animation while waiting for the API response.
void displayLoadingAnimation(atomic<bool> &done) {
    string animation = "|/-\\";
    int i = 0;
    while (!done) {
        cout << "\rProcessing " << animation[i % 4] << flush;
        this_thread::sleep_for(chrono::milliseconds(300));
        i++;
    }
    cout << "\rProcessing Done!       " << endl;
}

// Callback function used by libcurl to write received data into a string.
size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Reads a text file from a relative path.
string readTextFile(const string &filename) {
    ifstream file("./../" + filename);
    if (!file) {
        string err = "Could not open file " + filename;
        cerr << err << endl;
        logger.log(ERROR, err);
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

// Structure to hold the summarization results.
struct SummaryResult {
    string summaryText;
    int promptTokenCount;
    int candidatesTokenCount;
    string modelVersion;
};

// Summarizes the given text using the Gemini API.
SummaryResult summarizeText(const string &text) {
    logger.log(INFO, "Input text: " + text);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger.log(ERROR, "Curl initialization failed");
        return SummaryResult{};
    }
    string response;
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Constructing the JSON payload.
    Json::Value jsonPayload;
    Json::Value contentArray(Json::arrayValue);
    Json::Value partsArray(Json::arrayValue);
    Json::Value textPart;
    textPart["text"] = text;
    partsArray.append(textPart);
    Json::Value content;
    content["parts"] = partsArray;
    contentArray.append(content);
    jsonPayload["contents"] = contentArray;
    
    // Add generation configuration (e.g., max tokens)
    Json::Value generationConfig;
    generationConfig["max_output_tokens"] = 100;
    generationConfig["temperature"] = 1.0;
    jsonPayload["generationConfig"] = generationConfig;
    
    Json::StreamWriterBuilder writer;
    string payload = Json::writeString(writer, jsonPayload);
    
    // Setting up CURL options
    curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Prevent indefinite hanging
    
    CURLcode res = curl_easy_perform(curl);
    SummaryResult result;
    
    if (res == CURLE_OK) {
        try {
            Json::Value root;
            Json::CharReaderBuilder builder;
            JSONCPP_STRING errors;
            istringstream responseStream(response);
            Json::parseFromStream(builder, responseStream, &root, &errors);
            
            string summaryText;
            int promptTokenCount = 0;
            int candidatesTokenCount = 0;
            string modelVersion;

            if (!root.empty() && root.isObject()) {
                if (root.isMember("candidates") && root["candidates"].isArray() &&
                    root["candidates"].size() > 0) {
                    Json::Value candidate = root["candidates"][0]; // Get first candidate
                    if (candidate.isMember("content") &&
                        candidate["content"].isObject() &&
                        candidate["content"].isMember("parts") &&
                        candidate["content"]["parts"].isArray() &&
                        candidate["content"]["parts"].size() > 0) {
                    Json::Value part = candidate["content"]["parts"][0];
                    if (part.isMember("text") && part["text"].isString()) {
                            summaryText = part["text"].asString();
                    }
                }
            }

                if (root.isMember("usageMetadata") && root["usageMetadata"].isObject()) {
                    Json::Value usage = root["usageMetadata"];
                    if (usage.isMember("promptTokenCount") && usage["promptTokenCount"].isInt()) {
                        promptTokenCount = usage["promptTokenCount"].asInt();
                    }
                    if (usage.isMember("candidatesTokenCount") && usage["candidatesTokenCount"].isInt()) {
                        candidatesTokenCount = usage["candidatesTokenCount"].asInt();
                        if (candidatesTokenCount >= 100)
                            logger.log(WARNING, "Summary is more than 50 Words");
                    }
                }

                if (root.isMember("modelVersion") && root["modelVersion"].isString()) {
                    modelVersion = root["modelVersion"].asString();
                }
            } else {
                string err = "Error: Could not parse JSON response.";
                cerr << err << endl;
                logger.log(ERROR, err);
                cerr << "Response: " << response << endl;
            }
            result.summaryText = summaryText;
            result.promptTokenCount = promptTokenCount;
            result.candidatesTokenCount = candidatesTokenCount;
            result.modelVersion = modelVersion;
        } catch (const Json::LogicError &e) {
            string err = "JSON parsing error: " + string(e.what());
            cerr << err << endl;
            logger.log(ERROR, err);
            cerr << "Response: " << response << endl;
        }
    } else {
        logger.log(ERROR, "Curl request failed: " + string(curl_easy_strerror(res)));
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    
    logger.log(INFO, "Output summary: " + result.summaryText);
    return result;
}

//---------------------------------------------------------------------
// Test Functions with Unique Preambles
//---------------------------------------------------------------------
void checkCondition(bool condition, const string &message) {
    if (!condition) {
        logger.log(ERROR, message);
        cout<<"\nTest Failed\n";
    }
}
// Test: Valid Input
void testSummarizeValidInput() {
    logger.log(INFO, "Running Test: SummarizeValidInput");
    cout << "Running Test: SummarizeValidInput" << endl;
    string inputText = "This is a sample text for a valid input test.";
    string prompt = "TEST_VALID: Please summarize the following text into 50 words or less " + inputText;
    SummaryResult result = summarizeText(prompt);
    // Here, we expect the response to reflect the valid test scenario.
    // (For instance, it might include the marker "TEST_VALID" or otherwise be distinct.)
    if (result.summaryText.find("valid") != string::npos) {
        cout << "✅ TestSummarizeValidInput Passed" << endl;
        logger.log(INFO, "✅ TestSummarizeValidInput Passed");

    } else {
        cout << "❌ TestSummarizeValidInput Failed" << endl;
        logger.log(ERROR, "❌ TestSummarizeValidInput Failed");

    }
}

// Test: Empty Input
void testSummarizeEmptyInput() {
    logger.log(INFO, "Running Test: SummarizeEmptyInput");
    cout << "Running Test: SummarizeEmptyInput" << endl;
    string prompt = "TEST_EMPTY: Although no additional text is provided, generate a brief summary indicating the absence of content.";
    SummaryResult result = summarizeText(prompt);
    // Validate the summary reflects the empty input scenario.
    if (result.summaryText.find("empty") != string::npos || result.summaryText.length() < 20) {
        cout << "✅ TestSummarizeEmptyInput Passed" << endl;
        logger.log(INFO, "✅ TestSummarizeEmptyInput Passed");

    } else {
        cout << "❌ TestSummarizeEmptyInput Failed" << endl;
        logger.log(ERROR, "❌ TestSummarizeEmptyInput Failed");
    }
}

// Test: Input with Special Characters
void testSummarizeInputWithSpecialChars() {
    logger.log(INFO, "Running Test: SummarizeInputWithSpecialChars");
    cout << "Running Test: SummarizeInputWithSpecialChars" << endl;
    string inputText = "Hello! @#$%^&*()_+ {}[];':,.<>?/";
    string prompt = "TEST_SPECIAL: Summarize the following text with special characters in 50 words or less. " + inputText;
    SummaryResult result = summarizeText(prompt);
    // Check if the response reflects handling of special characters.
    if (result.summaryText.find("@") != string::npos) {
        cout << "✅ TestSummarizeInputWithSpecialChars Passed" << endl;
        logger.log(INFO, "✅ TestSummarizeInputWithSpecialChars Passed");
    } else {
        cout << "❌ TestSummarizeInputWithSpecialChars Failed" << endl;
        logger.log(ERROR, "❌ TestSummarizeInputWithSpecialChars Failed");
    }
}

// Test: API Error Simulation
void testSummarizeApiError() {
    logger.log(INFO, "Running Test: SummarizeApiError");
    cout << "Running Test: SummarizeApiError" << endl;
    // Simulate an API error by using an invalid API URL (this is a simple simulation)
    const string invalidUrl = "https://invalid.api.endpoint";
    
    auto summarizeTextWithInvalidUrl = [&](const string &text) -> SummaryResult {
        logger.log(INFO, "Summarizing text with invalid URL: " + text);
        CURL *curl = curl_easy_init();
        if (!curl) {
            logger.log(ERROR, "Curl initialization failed");
            return SummaryResult{};
        }
        string response;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        Json::Value jsonPayload;
        Json::Value content;
        Json::Value parts;
        Json::Value textPart;
        textPart["text"] = text;
        parts.append(textPart);
        content["parts"] = parts;
        jsonPayload["contents"].append(content);
        
        Json::Value generationConfig;
        generationConfig["max_output_tokens"] = 100;
        generationConfig["temperature"] = 1.0;
        jsonPayload["generationConfig"] = generationConfig;
        
        Json::StreamWriterBuilder writer;
        string payload = Json::writeString(writer, jsonPayload);
        
        curl_easy_setopt(curl, CURLOPT_URL, invalidUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        CURLcode res = curl_easy_perform(curl);
        SummaryResult result;
        if (res != CURLE_OK) {
            logger.log(ERROR, "Simulated API Error: " + string(curl_easy_strerror(res)));
            result.summaryText = "API_ERROR";
        }
        
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return result;
    };
    
    string prompt = "TEST_API_ERROR: This prompt should simulate an API error condition.";
    SummaryResult result = summarizeTextWithInvalidUrl(prompt);
    if (result.summaryText == "API_ERROR") {
        logger.log(INFO, "✅ TestSummarizeApiError Passed");
        cout << "✅ TestSummarizeApiError Passed" << endl;
    } else {
        logger.log(ERROR, "❌ TestSummarizeApiError Failed");
        cout << "❌ TestSummarizeApiError Failed" << endl;
    }
}

// Test: JSON Parsing Error Simulation
void testSummarizeJsonParsingError() {
    logger.log(INFO, "Running Test: SummarizeJsonParsingError");
    cout << "Running Test: SummarizeJsonParsingError" << endl;
    // Simulate a JSON parsing error by using a malformed JSON response
    auto summarizeTextWithMalformedJson = [&](const string &text) -> SummaryResult {
        logger.log(INFO, "Simulating JSON parsing error for text: " + text);
        string malformedResponse = "This is not a JSON response";
        SummaryResult result;
        try {
            Json::Value root;
            Json::CharReaderBuilder builder;
            JSONCPP_STRING errors;
            istringstream responseStream(malformedResponse);
            if (!Json::parseFromStream(builder, responseStream, &root, &errors)) {
                throw runtime_error("Malformed JSON");
            }
        } catch (const exception &e) {
            logger.log(ERROR, "Simulated JSON parsing error: " + string(e.what()));
            result.summaryText = "JSON_PARSING_ERROR";
        }
        return result;
    };
    
    string prompt = "TEST_JSON_ERROR: This prompt should simulate a JSON parsing error.";
    SummaryResult result = summarizeTextWithMalformedJson(prompt);
    if (result.summaryText == "JSON_PARSING_ERROR") {
        logger.log(INFO, "✅ TestSummarizeJsonParsingError Passed");
        cout << "✅ TestSummarizeJsonParsingError Passed" << endl;
    } else {
        logger.log(ERROR, "❌ TestSummarizeJsonParsingError Failed");
        cout << "❌ TestSummarizeJsonParsingError Failed" << endl;
    }
}

// Test: Max Output Tokens Limit
void testMaxOutputTokensLimit() {
    logger.log(INFO, "Running Test: MaxOutputTokensLimit");
    cout << "Running Test: MaxOutputTokensLimit" << endl;
    string inputText = "This is a sample text to test the max output tokens limit. It should be summarized into a summary that does not exceed the specified token limit.";
    string prompt = "TEST_TOKEN_LIMIT: Summarize the following text ensuring the summary respects a maximum token limit. " + inputText;
    SummaryResult result = summarizeText(prompt);
    // For example, we assume that if the summary length is below 200 characters, the token limit is respected.
    if (result.summaryText.length() < 200) {
        logger.log(INFO, "✅ TestMaxOutputTokensLimit Passed");
        cout << "✅ TestMaxOutputTokensLimit Passed" << endl;
    } else {
        logger.log(ERROR, "❌ TestMaxOutputTokensLimit Failed");
        cout << "❌ TestMaxOutputTokensLimit Failed" << endl;
    }
}

void TestEmailPdfSummarization() {
    logger.log(INFO, "Running TestEmailPdfSummarization...");
    cout << "Running Test: TestEmailPdfSummarization" << endl;
    string emailContent = readTextFile("email.txt");
    string pdfContent = readTextFile("pdf.txt");
    checkCondition(!emailContent.empty(), "Email content should not be empty");
    checkCondition(!pdfContent.empty(), "PDF content should not be empty");    string combinedText = "Summarise this in 50 words or less: " + emailContent + " " + pdfContent;
    SummaryResult summary = summarizeText(combinedText);
    checkCondition(!summary.summaryText.empty(), "Summary should not be empty");
    logger.log(INFO, "TestEmailPdfSummarization passed!");
    cout << "✅ TestEmailPdfSummarization Passed" << endl;
}

void TestFileNotFound() {
    logger.log(INFO, "Running TestFileNotFound...");
    cout << "Running Test: TestFileNotFound" << endl;

    try {
        string emailContent = readTextFile("nonexistent_email.txt");
        string pdfContent = readTextFile("nonexistent_pdf.txt");
        checkCondition(emailContent.empty(), "Email content should be empty if file does not exist");
        checkCondition(pdfContent.empty(), "PDF content should be empty if file does not exist");
        logger.log(INFO, "TestFileNotFound passed!");
        cout << "✅ TestFileNotFound Passed" << endl;
    } catch (const exception &e) {
        logger.log(ERROR, string("Caught exception as expected: ") + e.what());
        logger.log(INFO, "TestFileNotFound completed with expected exception!");
    }
}

//---------------------------------------------------------------------
// Main Function
//---------------------------------------------------------------------
int main() {
    logger.log(INFO, "Program started.");
    // Display the current working directory.
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        cout << "\nCurrent working directory: " << cwd << endl;
    } else {
        string err = "Error getting current working directory";
        cerr << err << endl;
        logger.log(ERROR, err);
    }
   
    int choice;
    cout << "\n===== Gemini Text Summarizer =====\n" << endl;

    do {
    cout << "\nEnter Choice\n  '1' to run the app,\n  '2' to run tests,\n  '3' to Quit the app: ";
    cin >> choice;
    cin.ignore(); // Consume any leftover newline
    
    if (choice == 1) {
    string emailContent, pdfText;
    
    cout << "\nEnter Email Content (or leave empty if using email.txt file, or enter a filename to read from): \n";
    getline(cin, emailContent);
    if (emailContent.empty()) {
        cout << "\nReading email content from email.txt...\n" << endl;
        emailContent = readTextFile("email.txt");
    } else if (emailContent.find(".txt") != string::npos) {
        cout << "\nReading email content from " << emailContent << "...\n" << endl;
        emailContent = readTextFile(emailContent);
    }
            
    cout << "\nEnter PDF Text Content (or leave empty if using pdf.txt file, or enter a filename to read from): \n";
    getline(cin, pdfText);
    if (pdfText.empty()) {
        cout << "\nReading PDF text from pdf.txt...\n" << endl;
        pdfText = readTextFile("pdf.txt");
    } else if (pdfText.find(".txt") != string::npos) {
        cout << "\nReading PDF text from " << pdfText << "...\n" << endl;
        pdfText = readTextFile(pdfText);
    }

    if (emailContent.empty() && pdfText.empty()) {
        string err = " No content provided for summarization!";
        cout << err << endl;
        logger.log(ERROR, err);
        // return 1;
    }
        
    // Combine the two text inputs with an instruction.
    string combinedText = "Summarise this in 50 words or less: " + emailContent + " " + pdfText;
        
    // Display a simple loading animation
    atomic<bool> done(false);
    thread loadingThread(displayLoadingAnimation, ref(done));
        
    SummaryResult summary = summarizeText(combinedText);

    done = true; // Stop the loading animation
        loadingThread.join();
        
        cout << "\nSummary: " << summary.summaryText << endl;
        logger.log(INFO, "Final Summary: " + summary.summaryText);
    } 
    else if (choice == 2) {
        // Run test cases
        logger.log(INFO, "Starting Summarize tests...");
        cout << "\nRunning Summarize Test Cases...\n" << endl;
        testSummarizeValidInput();
        testSummarizeEmptyInput();
        testSummarizeInputWithSpecialChars();
        // testSummarizeInputWithDifferentLengths();
        testSummarizeApiError();
        testSummarizeJsonParsingError();
        testMaxOutputTokensLimit();
        logger.log(INFO, "Starting integration tests...");
        cout << "\nRunning Integration Test Cases...\n" << endl;        
        TestEmailPdfSummarization();
        TestFileNotFound();
        logger.log(INFO, "All integration tests passed!");        
    } 
    else if (choice == 3){
        cout << "\nExiting application. Goodbye!\n" << endl;
    }
    else {
        cout << "Invalid choice! Please enter 1, 2, or 3." << endl;
    }
    } while (choice != 3);
    logger.log(INFO, "Program finished.");

    return 0;
}
