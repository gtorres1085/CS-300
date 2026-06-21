```cpp
//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Gustavo Torres
// Course      : CS-300
// Description : ABCU Advising Assistance Program
//============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

// Course object that stores one line of course data from the file.
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Binary search tree used to store courses by course number.
class BinarySearchTree {
private:
    struct Node {
        Course course;
        Node* left;
        Node* right;

        Node(const Course& aCourse) {
            course = aCourse;
            left = nullptr;
            right = nullptr;
        }
    };

    Node* root;

    void addNode(Node*& node, const Course& course) {
        if (node == nullptr) {
            node = new Node(course);
        }
        else if (course.courseNumber < node->course.courseNumber) {
            addNode(node->left, course);
        }
        else if (course.courseNumber > node->course.courseNumber) {
            addNode(node->right, course);
        }
        else {
            // If a duplicate somehow reaches insertion, update the existing course.
            node->course = course;
        }
    }

    Course* searchNode(Node* node, const string& courseNumber) const {
        if (node == nullptr) {
            return nullptr;
        }

        if (node->course.courseNumber == courseNumber) {
            return &(node->course);
        }
        else if (courseNumber < node->course.courseNumber) {
            return searchNode(node->left, courseNumber);
        }
        else {
            return searchNode(node->right, courseNumber);
        }
    }

    void inOrder(Node* node, vector<Course>& courses) const {
        if (node != nullptr) {
            inOrder(node->left, courses);
            courses.push_back(node->course);
            inOrder(node->right, courses);
        }
    }

    void destroy(Node* node) {
        if (node != nullptr) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

public:
    BinarySearchTree() {
        root = nullptr;
    }

    ~BinarySearchTree() {
        clear();
    }

    void clear() {
        destroy(root);
        root = nullptr;
    }

    bool isEmpty() const {
        return root == nullptr;
    }

    void insert(const Course& course) {
        addNode(root, course);
    }

    Course* search(const string& courseNumber) const {
        return searchNode(root, courseNumber);
    }

    vector<Course> getCoursesInOrder() const {
        vector<Course> courses;
        inOrder(root, courses);
        return courses;
    }
};

// Remove spaces from the beginning and end of a string.
string trim(const string& value) {
    size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }

    size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Convert course numbers to uppercase so searches are not case-sensitive.
string toUpperCase(string value) {
    transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(toupper(c));
        });
    return value;
}

// Remove surrounding quotation marks from a file name if the user enters them.
string removeSurroundingQuotes(const string& value) {
    string result = trim(value);
    if (result.length() >= 2 && result.front() == '"' && result.back() == '"') {
        return result.substr(1, result.length() - 2);
    }
    return result;
}

// Split one comma-separated line into trimmed tokens.
vector<string> splitLine(const string& line) {
    vector<string> tokens;
    string token;
    stringstream ss(line);

    while (getline(ss, token, ',')) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

// Load, parse, validate, and store the course data in the binary search tree.
bool loadCourses(const string& fileName, BinarySearchTree& courseTree) {
    ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        cout << "Error: File could not be opened." << endl;
        return false;
    }

    vector<vector<string>> parsedLines;
    unordered_set<string> validCourseNumbers;
    string line;
    int lineNumber = 0;

    // First pass: read each line, split it, and collect valid course numbers.
    while (getline(inputFile, line)) {
        lineNumber++;
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        vector<string> tokens = splitLine(line);

        if (tokens.size() < 2 || tokens.at(0).empty() || tokens.at(1).empty()) {
            cout << "Error: Line " << lineNumber
                << " must include a course number and course title." << endl;
            return false;
        }

        tokens.at(0) = toUpperCase(tokens.at(0));

        if (validCourseNumbers.count(tokens.at(0)) > 0) {
            cout << "Error: Duplicate course number found: " << tokens.at(0) << endl;
            return false;
        }

        validCourseNumbers.insert(tokens.at(0));
        parsedLines.push_back(tokens);
    }

    inputFile.close();

    // Second pass: verify that every nonblank prerequisite exists in the file.
    for (vector<string>& tokens : parsedLines) {
        for (size_t i = 2; i < tokens.size(); ++i) {
            tokens.at(i) = toUpperCase(trim(tokens.at(i)));

            // The provided CSV contains trailing commas, so blank prerequisite fields are ignored.
            if (tokens.at(i).empty()) {
                continue;
            }

            if (validCourseNumbers.count(tokens.at(i)) == 0) {
                cout << "Error: Prerequisite " << tokens.at(i)
                    << " does not exist as a course in the file." << endl;
                return false;
            }
        }
    }

    // If the file is valid, clear old data and insert one Course object per line.
    courseTree.clear();

    for (const vector<string>& tokens : parsedLines) {
        Course course;
        course.courseNumber = tokens.at(0);
        course.courseTitle = tokens.at(1);

        for (size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens.at(i).empty()) {
                course.prerequisites.push_back(tokens.at(i));
            }
        }

        courseTree.insert(course);
    }

    cout << "Course data loaded successfully." << endl;
    return true;
}

// Print all courses in alphanumeric order using an in-order traversal.
void printCourseList(const BinarySearchTree& courseTree) {
    if (courseTree.isEmpty()) {
        cout << "No course data has been loaded." << endl;
        return;
    }

    vector<Course> courses = courseTree.getCoursesInOrder();

    cout << "Here is a sample schedule:" << endl;
    for (const Course& course : courses) {
        cout << course.courseNumber << ", " << course.courseTitle << endl;
    }
}

// Print one course and its prerequisites.
void printCourseInformation(const BinarySearchTree& courseTree, const string& courseNumber) {
    if (courseTree.isEmpty()) {
        cout << "No course data has been loaded." << endl;
        return;
    }

    Course* course = courseTree.search(toUpperCase(courseNumber));

    if (course == nullptr) {
        cout << "Course not found." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->courseTitle << endl;

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    cout << "Prerequisites: ";
    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        Course* prerequisite = courseTree.search(course->prerequisites.at(i));

        if (prerequisite != nullptr) {
            cout << prerequisite->courseNumber << " (" << prerequisite->courseTitle << ")";
        }
        else {
            cout << course->prerequisites.at(i);
        }

        if (i < course->prerequisites.size() - 1) {
            cout << ", ";
        }
    }
    cout << endl;
}

void displayMenu() {
    cout << endl;
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << "What would you like to do? ";
}

int main() {
    BinarySearchTree courseTree;
    bool dataLoaded = false;
    string choice;

    cout << "Welcome to the course planner." << endl;

    while (choice != "9") {
        displayMenu();
        getline(cin, choice);
        choice = trim(choice);

        if (choice == "1") {
            string fileName;
            cout << "Enter the course data file name: ";
            getline(cin, fileName);
            fileName = removeSurroundingQuotes(fileName);

            dataLoaded = loadCourses(fileName, courseTree);
        }
        else if (choice == "2") {
            if (!dataLoaded) {
                cout << "Please load the course data first." << endl;
            }
            else {
                printCourseList(courseTree);
            }
        }
        else if (choice == "3") {
            if (!dataLoaded) {
                cout << "Please load the course data first." << endl;
            }
            else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                printCourseInformation(courseTree, trim(courseNumber));
            }
        }
        else if (choice == "9") {
            cout << "Thank you for using the course planner!" << endl;
        }
        else {
            cout << choice << " is not a valid option." << endl;
        }
    }

    return 0;
}
```
