// #include "gtest/gtest.h"
// #include "../App.cpp" // Adjust path as needed
// #include <string>
// #include <sstream>
// #include <fstream>
// #include <stdexcept> // Include for runtime_error

// // Mock API response function (using a string)
// std::string mockApiResponse(const std::string& jsonResponse) {
//     return jsonResponse;
// }
// typedef CURLcode (*curl_easy_perform_type)(CURL*);
// curl_easy_perform_type real_curl_easy_perform = curl_easy_perform; // Store the real function

// TEST(SummarizeTextTest, TestSummarizeValidInput) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";

//     // Mock the API Call
//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult result = summarizeText("Test input");

//     curl_easy_perform = original_curl_easy_perform;
//     EXPECT_EQ(result.summaryText, "Mocked summary");
//     EXPECT_EQ(result.promptTokenCount, 10);
//     EXPECT_EQ(result.candidatesTokenCount, 5);
//     EXPECT_EQ(result.modelVersion, "gemini-2.0-flash");

// }

// TEST(SummarizeTextTest, TestSummarizeEmptyInput) {
//     SummaryResult result = summarizeText("");
//     EXPECT_TRUE(result.summaryText.empty());
// }

// TEST(SummarizeTextTest, TestSummarizeInputWithSpecialChars) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary with special chars"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";

//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult result = summarizeText("Input with \n newlines \t tabs and Unicode: äöü.");

//     curl_easy_perform = original_curl_easy_perform;

//     EXPECT_EQ(result.summaryText, "Mocked summary with special chars");
// }

// TEST(SummarizeTextTest, TestSummarizeInputWithDifferentLengths) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";

//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult shortResult = summarizeText("short");
//     SummaryResult mediumResult = summarizeText("medium length input");
//     SummaryResult longResult = summarizeText("long input text to test");

//     curl_easy_perform = original_curl_easy_perform;

//     EXPECT_FALSE(shortResult.summaryText.empty());
//     EXPECT_FALSE(mediumResult.summaryText.empty());
//     EXPECT_FALSE(longResult.summaryText.empty());
// }

// TEST(SummarizeTextTest, TestSummarizeApiError) {
//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_FAILED_INIT; }; // Simulate a CURL error

//     EXPECT_THROW({summarizeText("Test input");}, std::runtime_error);

//     curl_easy_perform = original_curl_easy_perform;
// }

// TEST(SummarizeTextTest, TestSummarizeJsonParsingError) {
//     std::string mockResponse = "This is not valid JSON";

//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     EXPECT_THROW({summarizeText("Test input");}, std::runtime_error);

//     curl_easy_perform = original_curl_easy_perform;
// }

// TEST(SummarizeTextTest, TestTokenCountCheck) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";
//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult result = summarizeText("Test input");

//     curl_easy_perform = original_curl_easy_perform;

//     EXPECT_LE(result.promptTokenCount, 5000);
//     EXPECT_LE(result.candidatesTokenCount, 100);
// }

// TEST(SummarizeTextTest, TestMaxOutputTokensLimit) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";
//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult result = summarizeText("Long input to test max output tokens");

//     curl_easy_perform = original_curl_easy_perform;

//     EXPECT_LE(result.summaryText.length(), 100); // Check word count (adjust 100 as needed)
// }

// TEST(SummarizeTextTest, TestModelVersionCheck) {
//     std::string mockResponse = R"({"candidates": [{"content": {"parts": [{"text": "Mocked summary"}]}}], "usageMetadata": {"promptTokenCount": 10, "candidatesTokenCount": 5}, "modelVersion": "gemini-2.0-flash"})";
//     auto original_curl_easy_perform = curl_easy_perform;
//     curl_easy_perform = [](CURL*){ return CURLE_OK; };

//     SummaryResult result = summarizeText("Test input");

//     curl_easy_perform = original_curl_easy_perform;

//     EXPECT_EQ(result.modelVersion, "gemini-2.0-flash");
// }