# Gemini Summarizer

## Overview

The **Gemini Summarizer** is a C++ command-line application that leverages the Gemini API to generate concise summaries from text inputs. It is designed to handle inputs from various sources—such as emails and PDFs—and comes equipped with a suite of tests to validate its functionality. Additionally, the application includes a logging mechanism to record events, errors, and test outcomes.

## Features

- **Text Summarization:** Uses the Gemini API to produce concise summaries.
- **Interactive CLI:** Offers a menu-driven interface for running the summarizer or tests.
- **Loading Animation:** Displays an animation while waiting for API responses.
- **Logging:** Logs key events and errors to a log file.
- **Robust Testing:** Includes tests for:
  - Valid inputs
  - Empty inputs
  - Inputs with special characters
  - API error simulations
  - JSON parsing error simulations
  - Maximum output token limit enforcement
  - File integration (email and PDF content)
  - File-not-found scenarios

## Prerequisites

- **C++ Compiler:** Must support C++17.
- **CMake:** Version 3.10 or later.
- **cURL:** Library for making HTTP requests.
- **JsonCpp:** Library for JSON parsing and handling.

## Build Instructions

1. **Clone the Repository**

```bash
git clone https://github.com/your-username/cpp-email-summarizer.git
cd cpp-email-summarizer
```
2. **Create a Build Directory and Generate Build Files and Compile the Application**

```bash
mkdir build
cd build
cmake ..
make
```
- This process generates an executable named App.
- If the Build already exists use : 
```bash 
rm -rf build        #Run in the Root Directory
```
## Running the Application

- Run the executable from the build directory:

```bash
./App
```
- Upon running, you will be presented with a menu:

- 1: Run the summarizer (prompts for email and PDF content, or reads from text files).
- 2: Execute the integrated test cases.
- 3: Quit the application.

## Test Cases

### The application provides multiple tests to ensure robust functionality:

- Valid Input Test: Verifies summarization with a proper input text.
- Empty Input Test: Checks behavior when no content is provided.
- Special Characters Test: Ensures correct handling of inputs containing special characters.
- API Error Simulation: Tests error handling by simulating a faulty API endpoint.
- JSON Parsing Error Simulation: Simulates and catches JSON parsing errors.
- Max Output Tokens Limit Test: Ensures that the summary does not exceed a predefined token limit.
- Integration Tests:
- TestEmailPdfSummarization: Combines content from email.txt and pdf.txt for summarization.
- TestFileNotFound: Validates behavior when expected input files are missing.

## Configuration

- API Key & Endpoint:
- The application uses a hardcoded API key and endpoint for the Gemini API. You can update these values in App.cpp as needed:
```bash
const string API_KEY = "YOUR_API_KEY"; // use your own API_KEY
const string API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + API_KEY;
```
## Logging

The application logs input text, output summaries, and any errors to the `summarizer-app.log` file in the same directory as the executable.- Log File:
- All logs are written to
  ```bash
  ../summarizer-app.log. #Adjust the path if needed.
  ```



# 📂 Project Structure

```bash
.📂 email-ai-summarizer
├── 📂 build            # Build of Main application code and test cases available only after you create it
├── 📄 App.cpp          # Main application code and test cases
├── 📄 CMakeLists.txt   # CMake build configuration
├── 📄 email.txt        # (Optional) Sample email text input
├── 📄 pdf.txt          # (Optional) Sample PDF text input
└── 📄 README.md        # Project documentation
```
## Troubleshooting
- cURL Initialization Errors: Ensure the cURL library is installed and correctly linked.
- JsonCpp Issues: Confirm that the JsonCpp library is installed.
- File Not Found: Verify that email.txt and pdf.txt are in the expected directory relative to the executable.
- API Key Errors: Make sure the API key and endpoint are valid.

## License
- This project is provided "as is" without any warranty. 

## Contributing
- Contributions are welcome! If you wish to improve or extend the project, please fork the repository and submit a pull request with your changes.

## Acknowledgements
- cURL
- JsonCpp
- CMake
