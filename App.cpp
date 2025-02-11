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
#include <iomanip>     // For put_time
#include <cctype>      // For tolower

using namespace std;

//---------------------------------------------------------------------
// Logger Class Definition
//---------------------------------------------------------------------

// Enum to represent log levels
enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

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
        cout << logEntry.str();

        // Output to log file if it's open
        if (logFile.is_open()) {
            logFile << logEntry.str();
            logFile.flush(); // Ensure immediate write to file
        }
    }

private:
    ofstream logFile; // File stream for the log file

    // Converts log level to a string for output
    string levelToString(LogLevel level) {
        switch(level) {
            case DEBUG:    return "DEBUG";
            case INFO:     return "INFO";
            case WARNING:  return "WARNING";
            case ERROR:    return "ERROR";
            case CRITICAL: return "CRITICAL";
            default:       return "UNKNOWN";
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
        string err = "Error: Could not open file " + filename;
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

    // Adding generation configuration
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

    if (res != CURLE_OK) {
        string err = "Curl request failed: " + string(curl_easy_strerror(res));
        cerr << err << endl;
        logger.log(ERROR, err);
        response = err;
    }

    if (res == CURLE_OK) {
        try {
            Json::Value root;
            Json::CharReaderBuilder builder;
            JSONCPP_STRING errors;
            std::istringstream responseStream(response);
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
        string err = "Curl request failed: " + string(curl_easy_strerror(res));
        cerr << err << endl;
        logger.log(ERROR, err);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.log(INFO, "Output summary: " + result.summaryText);
    return result;
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

    string emailContent, pdfText;

    cout << "\n===== Gemini Text Summarizer =====\n" << endl;

    cout << "\nEnter Email Content (or leave empty if using email.txt file): \n";
    getline(cin, emailContent);
    if (emailContent.empty()) {
        cout << "\nReading email content from email.txt...\n" << endl;
        emailContent = readTextFile("email.txt");
    }

    cout << "\nEnter PDF Text Content (or leave empty if using pdf.txt file): \n";
    getline(cin, pdfText);
    if (pdfText.empty()) {
        cout << "\nReading PDF text from pdf.txt...\n" << endl;
        pdfText = readTextFile("pdf.txt");
    }

    if (emailContent.empty() && pdfText.empty()) {
        string err = "Error: No content provided for summarization!";
        cout << err << endl;
        logger.log(ERROR, err);
        return 1;
    }

    // Combine the two text inputs with an instruction.
    string combinedText = "Summarise this in 50 words or less: " + emailContent + " " + pdfText;

    cout << "\nSummarizing...\n" << endl;
    atomic<bool> done(false);
    thread loadingThread(displayLoadingAnimation, ref(done));

    SummaryResult summary = summarizeText(combinedText);

    done = true; // Stop the loading animation
    loadingThread.join();

    cout << "\nSummary: " << summary.summaryText << endl;
    logger.log(INFO, "Final Summary: " + summary.summaryText);
    logger.log(INFO, "Program finished.");

    return 0;
}
