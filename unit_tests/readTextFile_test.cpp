// #include "gtest/gtest.h" // Assuming you're using Google Test
// #include "../App.cpp"       // Include your App.cpp file (adjust path as needed)
// #include <fstream>
// #include <string>

// TEST(ReadTextFileTest, TestReadFileExists) {
//     std::ofstream("test_data/valid_input.txt") << "This is valid input text."; // Create test file
//     std::string content = readTextFile("test_data/valid_input.txt");
//     EXPECT_EQ(content, "This is valid input text.");
//     std::remove("test_data/valid_input.txt"); // Clean up
// }

// TEST(ReadTextFileTest, TestReadFileDoesNotExist) {
//     std::string content = readTextFile("nonexistent_file.txt");
//     EXPECT_TRUE(content.empty());
// }

// TEST(ReadTextFileTest, TestReadEmptyFile) {
//     std::ofstream("test_data/empty_file.txt"); // Create empty test file
//     std::string content = readTextFile("test_data/empty_file.txt");
//     EXPECT_TRUE(content.empty());
//     std::remove("test_data/empty_file.txt"); // Clean up
// }

// TEST(ReadTextFileTest, TestReadFileWithSpecialChars) {
//     std::ofstream("test_data/special_chars.txt") << "This has \n newlines \t tabs and Unicode: \u00E4\u00F6\u00FC.";
//     std::string content = readTextFile("test_data/special_chars.txt");
//     EXPECT_EQ(content, "This has \n newlines \t tabs and Unicode: äöü.");
//     std::remove("test_data/special_chars.txt"); // Clean up
// }

// TEST(ReadTextFileTest, TestReadLargeFile) {
//     std::ofstream("test_data/large_file.txt");
//     std::ofstream largeFile("test_data/large_file.txt");
//     for (int i = 0; i < 10000; ++i) { // Example: 10,000 lines
//         largeFile << "Neque porro quisquam est qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit " << i << std::endl;
//     }
//     largeFile.close();
//     std::string content = readTextFile("test_data/large_file.txt");
//     EXPECT_FALSE(content.empty()); // Just check it's not empty
//     std::remove("test_data/large_file.txt"); // Clean up
// }