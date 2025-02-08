#include <iostream>
#include <string>
#include <thread>
#include <chrono>
// #include "api.h"
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

const string API_KEY = "AIzaSyCpR7hiVbA6PVjz-rXyJ69NZ4k2QrsnJY4";
const string API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=AIzaSyCpR7hiVbA6PVjz-rXyJ69NZ4k2QrsnJY4";

void displayLoadingAnimation() {
    string animation = "|/-\\";
    for (int i = 0; i < 10; i++) {
        cout << "\rProcessing " << animation[i % 4] << flush;
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
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
    string API_KEY = "AIzaSyCpR7hiVbA6PVjz-rXyJ69NZ4k2QrsnJY4";
    string API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=AIzaSyCpR7hiVbA6PVjz-rXyJ69NZ4k2QrsnJY4";

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + API_KEY).c_str());

    Json::Value jsonPayload;
    jsonPayload["text"] = text;
    jsonPayload["max_length"] = 50;
    Json::StreamWriterBuilder writer;
    string payload = Json::writeString(writer, jsonPayload);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "Curl request failed: " << curl_easy_strerror(res) << endl;
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
    thread loadingThread(displayLoadingAnimation);
    
    string summary = summarizeText(combinedText);
    
    loadingThread.detach();
    cout << "\nSummary: " << summary << endl;
    
    return 0;
}

