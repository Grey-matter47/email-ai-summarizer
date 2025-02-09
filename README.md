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
