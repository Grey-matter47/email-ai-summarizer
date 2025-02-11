#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <curl/curl.h>
#include <unistd.h> // For getcwd()
#include <limits.h> // For PATH_MAX
#include <json/json.h>
#include <atomic> // Include for atomic<bool>
#include <ctime>   // For time_t, time, and ctime
#include <iomanip> // For put_time

using namespace std;

const string API_KEY = "AIzaSyBrWYPnBdGi2KC0DfiuxBgyBrnvlcMASqQ";
const string API_URL =
    "https://generativelanguage.googleapis.com/v1beta/models/"
    "gemini-2.0-flash:generateContent?key=" +
    API_KEY;

// Global log file stream
fstream logFile("summarizer-app.log", ios::app);

// Function to log messages with timestamp
void logMessage(const string &message) {
    if (!logFile.is_open()) {  // Check if the file is open
        logFile.open("/tmp/summarizer-app.log", ios::app); // Open it if it's not
        if (!logFile.is_open()) { // Check if file could be opened
            cerr << "Error: Could not open log file." << endl;
            return;
        }
    }
    cout << "logMessage called!" << endl; 
    time_t currentTime = time(0);
    logFile << put_time(localtime(&currentTime), "%F %T") << " - " << message << endl;
    logFile.flush();
    // cout << "Just checking if its written in logfile!" << currentTime<<"   "<< message<< endl; 

}

void displayLoadingAnimation(atomic<bool> &done) {
  string animation = "|/-\\";
  int i = 0;
  while (!done) {
    cout << "\rProcessing " << animation[i % 4] << flush;
    this_thread::sleep_for(chrono::milliseconds(300));
    i++;
  }
  cout << "\rProcessing Done!       " << endl; // Clear the animation
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output) {
  size_t totalSize = size * nmemb;
  output->append(static_cast<char *>(contents), totalSize); // Use static_cast
  return totalSize;
}

string readTextFile(const string &filename) {
  ifstream file("./../" + filename); // Replace with the actual path
  cout << "\nWassup bitch still not\n";
  if (!file) {
    cerr << "Error: Could not open file " << filename << endl;
    logMessage("Error: Could not open file " + filename);
    return "";
  }
  string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
  return content;
}

struct SummaryResult {
    string summaryText;
    int promptTokenCount;
    int candidatesTokenCount;
    string modelVersion;
};

SummaryResult summarizeText(const string &text) {
  logMessage("Input text: " + text); // Log input text

  CURL *curl = curl_easy_init();
    if (!curl) {
        logMessage("Curl initialization failed");
        // Return an empty SummaryResult to indicate failure.
        return SummaryResult{}; // This creates a default-initialized SummaryResult
    }
  string response;
  struct curl_slist *headers = nullptr;

  headers = curl_slist_append(headers, "Content-Type: application/json");

  // Constructing the JSON payload
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

  // Adding the token limit
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
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Setting a timeout to prevent indefinite hanging

  CURLcode res = curl_easy_perform(curl);
  SummaryResult result;

  if (res!= CURLE_OK) {
    cerr << "Curl request failed: " << curl_easy_strerror(res) << endl;
    logMessage("Curl request failed: " + string(curl_easy_strerror(res)));
    response = "Curl request failed: " + string(curl_easy_strerror(res));
  }

  if (res == CURLE_OK) {
    try {
      Json::Value root;
      Json::CharReaderBuilder builder;
      Json::String errors;
      std::istringstream responseStream(response);
      Json::parseFromStream(builder, responseStream, &root, &errors);
    string summaryText;
    int promptTokenCount = 0;
    int candidatesTokenCount = 0;
    string modelVersion;

      if (!root.empty() && root.isObject()) {
        if (root.isMember("candidates") && root["candidates"].isArray() &&
            root["candidates"].size() > 0) {
          Json::Value candidate = root["candidates"][0]; // Get the first candidate
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

        if (root.isMember("usageMetadata") &&
            root["usageMetadata"].isObject()) {
          Json::Value usage = root["usageMetadata"];
          if (usage.isMember("promptTokenCount") &&
              usage["promptTokenCount"].isInt()) {
            promptTokenCount = usage["promptTokenCount"].asInt();
          }
          if (usage.isMember("candidatesTokenCount") &&
              usage["candidatesTokenCount"].isInt()) {
            candidatesTokenCount = usage["candidatesTokenCount"].asInt();
            if (candidatesTokenCount >= 100)
              cerr << "Summary is more than 50 Words" << endl;
          }
        }
        if (root.isMember("modelVersion") && root["modelVersion"].isString()) {
          modelVersion = root["modelVersion"].asString();
        }
      } else {
        cerr << "Error: Could not parse JSON response." << endl;
        logMessage("Error: Could not parse JSON response.");
        cerr << "Response: " << response << endl; // Print the full response for // debugging
      }
   result.summaryText = summaryText;
   result.promptTokenCount = promptTokenCount;
   result.candidatesTokenCount = candidatesTokenCount;
   result.modelVersion = modelVersion;

    } catch (const Json::LogicError &e) {
      cerr << "JSON parsing error: " << e.what() << endl;
      logMessage("JSON parsing error: " + string(e.what()));
      cerr << "Response: " << response << endl; // Print the full response for debugging
    }
  }  else {
        cerr << "\nCurl request failed: \n" << curl_easy_strerror(res) << endl;
        logMessage("Curl request failed: " + string(curl_easy_strerror(res)));
        cerr<<"\nCurl request failed\n"; 
    }

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  logMessage("Output summary: " + result.summaryText); // Log output summary  
  
  return result;                          // Return the extracted summary text
}
//     return response;

int main() {
    // Opening the log file here, at the beginning of main
    logFile.open("summarizerApp.log", ios::app);
    cout<<"We are on like babylon\n";
    if (!logFile.is_open()) {
        char er;
        cerr << "\nError: Could not open log file, logging not possible do you still want to proceed(Y/N):\n" << endl;
        cin >> er;
        if (tolower(er) != 'y') { 
            cerr << "\nProgram terminated by user.\n" << endl;
            exit(1); // Exit the program immediately
        }
        // If the user chooses to proceed, we still don't have logging.
    }
    
    SummaryResult summaryResult; 

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd))!= nullptr) {
    cout << "\nCurrent working directory: \n"
         << cwd << std::endl;
  } else {
    cerr << "\nError getting current working directory\n" << std::endl;
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
    cout << "\nError: No content provided for summarization!\n" << endl;
    logMessage("Error: No content provided for summarization!");
    return 1;
  }

  string combinedText =
      "Summarise this in 50 word or less: " + emailContent + " " + pdfText;

  cout << "\nSummarizing...\n" << endl;
  atomic<bool> done(false);
  thread loadingThread(displayLoadingAnimation, ref(done));

  SummaryResult summary = summarizeText(combinedText);

  done = true; // Signal the loading thread to stop
  loadingThread.join();

  cout << "\nSummary: " << summary.summaryText << endl;
  // Log the summary
  logMessage("Final Summary: " + summary.summaryText);
      
if (logFile.is_open())
    logFile.close();
  return 0;
}