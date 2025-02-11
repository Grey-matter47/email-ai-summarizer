// #include "gtest/gtest.h"
// #include "../App.cpp"
// #include <fstream>
// #include <string>

// TEST(IntegrationTest, TestEmailPdfSummarization) {
//     std::ofstream("test_data/email.txt") << "Test email content.";
//     std::ofstream("test_data/pdf.txt") << "Test PDF content.";
//     std::string summary = summarizeText("Summarise this in 50 word or less : Test email content. Test PDF content.");
//     EXPECT_FALSE(summary.empty());
//     std::remove("test_data/email.txt");
//     std::remove("test_data/pdf.txt");
// }

// TEST(IntegrationTest, TestEmptyFiles) {
//     std::ofstream("test_data/email.txt"); // Empty email
//     std::ofstream("test_data/pdf.txt");   // Empty PDF
//     std::string summary = summarizeText("Summarise this in 50 word or less : "); // Empty input
//     EXPECT_TRUE(summary.empty());
//     std::remove("test_data/email.txt");
//     std::remove("test_data/pdf.txt");
// }

// TEST(IntegrationTest, TestFileNotFound) {
//     std::string summary = summarizeText("Summarise this in 50 word or less : "); // Files don't exist
//     EXPECT_TRUE(summary.empty()); 
// }