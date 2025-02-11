# email-ai-summarizer
Building a C++ App to Summarize Emails with Gemini 2.0 Flash
# C++ Email Summarizer with Gemini 2.0 Flash
- A C++ application that summarizes an email and attached PDF (as .txt) in ≤ 50 words using the Gemini 2.0 Flash API.
- Includes unit tests (Google Test) and logs input/output for validation.

# Features
- ✔ Reads email text (email.txt) and attachment text (attachment.txt)
- ✔ Calls Gemini 2.0 Flash API for AI-powered summarization
- ✔ Limits summary output to ≤ 50 words
- ✔ Logs inputs & outputs for traceability (logs/input_log.txt, logs/output_log.txt)
- ✔ Includes unit tests for correctness (gtest framework)
- ✔ Automated CI tests on GitHub

# 📂 Project Structure
```bash
📂 cpp-email-summarizer
│── 📄 main.cpp           # Main program
│── 📄 gemini_api.cpp     # Handles API calls
│── 📄 gemini_api.h       # API function declarations
│── 📄 file_handler.cpp   # Reads email & PDF (txt)
│── 📄 file_handler.h     # File handling declarations
│── 📄 tests.cpp          # Unit tests (Google Test)
│── 📂 data
│   ├── email.txt         # Sample email input
│   ├── attachment.txt    # Sample attachment input
│── 📂 logs
│   ├── input_log.txt     # Stores input logs
│   ├── output_log.txt    # Stores API response summaries
│── 📄 CMakeLists.txt     # Build system config
│── 📄 README.md          # Instructions & Setup
```
# Installation & Setup
- 1 Clone the Repository
```bash
git clone https://github.com/your-username/cpp-email-summarizer.git
cd cpp-email-summarizer
```

# Unit Tests for Text Summarizer

This directory contains unit and integration tests for the C++ text summarizer application.

## Running Tests

To run the tests, you'll need a C++ testing framework (like Google Test).  Compile and link the test files with the testing framework and your application code.  Then, execute the resulting test executable.

## Test Files

### `readTextFile_test.cpp`

*   **`TestReadFileExists`:** Tests `readTextFile` with a valid filename (`test_data/valid_input.txt`). Verifies that the function reads the file content correctly.
*   **`TestReadFileDoesNotExist`:** Tests `readTextFile` with a non-existent filename. Verifies that the function returns an empty string and logs an error message.
*   **`TestReadEmptyFile`:** Tests `readTextFile` with an empty file (`test_data/empty_file.txt`). Verifies that an empty string is returned and no error is logged.
*   **`TestReadFileWithSpecialChars`:** Tests `readTextFile` with a file containing special characters (`test_data/special_chars.txt`). Verifies correct handling of special characters.
*   **`TestReadLargeFile`:** Tests `readTextFile` with a large file (`test_data/large_file.txt`). Verifies that the function can handle large files without issues.

### `summarizeText_test.cpp`

*   **`TestSummarizeValidInput`:** Tests `summarizeText` with valid input text.  Uses a mocked API response.  Verifies that a non-empty summary is returned.
*   **`TestSummarizeEmptyInput`:** Tests `summarizeText` with an empty input string. Verifies that the function handles this gracefully.
*   **`TestSummarizeInputWithSpecialChars`:** Tests `summarizeText` with input containing special characters. Uses a mocked API response. Verifies correct handling of special characters.
*   **`TestSummarizeInputWithDifferentLengths`:** Tests `summarizeText` with short, medium, and long input texts. Uses mocked API responses.
*   **`TestSummarizeApiError`:**  Tests `summarizeText` with a mocked API error response. Verifies that an appropriate error message is logged.
*   **`TestSummarizeJsonParsingError`:** Tests `summarizeText` with a mocked malformed JSON response. Verifies that a JSON parsing error is logged.
*   **`TestTokenCountCheck`:**  Tests if the `promptTokenCount` and `candidatesTokenCount` in the response are within reasonable limits. Uses a mocked API response.
*   **`TestMaxOutputTokensLimit`:**  Tests if the `max_output_tokens` parameter is respected. Uses a mocked API response.
*   **`TestModelVersionCheck`:**  Tests if the returned `modelVersion` matches the expected value. Uses a mocked API response.

### `integration_test.cpp`

*   **`TestEmailPdfSummarization`:** Tests the entire summarization flow with sample `email.txt` and `pdf.txt` files.
*   **`TestEmptyFiles`:** Tests the integration when either or both input files are empty.
*   **`TestFileNotFound`:** Tests the integration when one or both input files do not exist.

## Logging

The application logs input text, output summaries, and any errors to the `summarizer-app.log` file in the same directory as the executable.