#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>
#include <atomic> // Include for atomic<bool>

using namespace std;

const string API_KEY = "AIzaSyBrWYPnBdGi2KC0DfiuxBgyBrnvlcMASqQ"; 
const string API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + API_KEY;

void displayLoadingAnimation(atomic<bool>& done) {
    string animation = "|/-\\";
    int i = 0;
    while (!done) {
        cout << "\rProcessing " << animation[i % 4] << flush;
        this_thread::sleep_for(chrono::milliseconds(300));
        i++;
    }
    cout << "\rProcessing Done!       " << endl; // Clear the animation
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize); // Use static_cast
    return totalSize;
}

string readTextFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

string summarizeText(const string& text) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Curl initialization failed";

    string response;
    struct curl_slist* headers = nullptr;

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
    generationConfig["max_output_tokens"] = 50;
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "Curl request failed: " << curl_easy_strerror(res) << endl;
        response = "Curl request failed: " + string(curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return response;
}

int main() {
    string emailContent, pdfText;

    cout << "===== Gemini Text Summarizer =====" << endl;

    cout << "Enter Email Content (or leave empty if using a file): ";
    getline(cin, emailContent);

    if (emailContent.empty()) {
        cout << "Reading email content from email.txt..." << endl;
        emailContent = readTextFile("email.txt");
    }

    cout << "Enter PDF Text Content (or leave empty if using a file): ";
    getline(cin, pdfText);

    if (pdfText.empty()) {
        cout << "Reading PDF text from pdf.txt..." << endl;
        pdfText = readTextFile("pdf.txt");
    }

    if (emailContent.empty() && pdfText.empty()) {
        cout << "Error: No content provided for summarization!" << endl;
        return 1;
    }

    string combinedText = emailContent + " " + pdfText;

    cout << "Summarizing..." << endl;
    atomic<bool> done(false);
    thread loadingThread(displayLoadingAnimation, ref(done));

    string summary = summarizeText(combinedText);

    done = true; // Signal the loading thread to stop
    loadingThread.join(); // Wait for the loading thread to finish

    cout << "\nSummary: " << summary << endl;

    return 0;
}