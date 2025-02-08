// #include <iostream>
// #include <fstream>
// #include <string>
// #include <curl/curl.h>
// #include <json/json.h>
// #include "../api.h"
// #include "../config.h"

// using namespace std;

// size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
//     size_t totalSize = size * nmemb;
//     output->append((char*)contents, totalSize);
//     return totalSize;
// }

// string readTextFile(const string& filename) {
//     ifstream file(filename);
//     if (!file) {
//         cerr << "Error: Could not open file " << filename << endl;
//         return "";
//     }
//     string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
//     return content;
// }

// string summarizeText(const string& text) {
//     CURL* curl = curl_easy_init();
//     if (!curl) return "Curl initialization failed";

//     string response;
//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, "Content-Type: application/json");
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + API_KEY).c_str());

//     Json::Value jsonPayload;
//     jsonPayload["text"] = text;
//     jsonPayload["max_length"] = 50;
//     Json::StreamWriterBuilder writer;
//     string payload = Json::writeString(writer, jsonPayload);

//     curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_POST, 1);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
//     CURLcode res = curl_easy_perform(curl);
//     if (res != CURLE_OK) {
//         cerr << "Curl request failed: " << curl_easy_strerror(res) << endl;
//     }

//     curl_easy_cleanup(curl);
//     curl_slist_free_all(headers);
//     return response;
// }
