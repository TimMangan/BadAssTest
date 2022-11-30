# BadAssTest

BadAssTest is a purpose built C++ Windows Console Test application that can be used to cause specific patterns of Windows API File I/O calls.
This is useful in testing runtime environments, shims, and filter mode drivers.

The app currently has a set of 6 built in API calls:
| Type Number | API | Purpose |
| ----------- | --- | ------- |
| 1 | std::system::filesystem::exists() | Test if a file exists |
| 2 | GetFileAttributes() | Test for existance and return file/directory attributes |
| 3 | CreateFile() | Test 3 tests for read-only access to the file, and to only open existing files |
| 4 | CreateFile() | Test 4 tests for read-write access, and only existing files |
| 5 | DeleteFile() | Test if we can delete a file. |
| 6 | FindXXFile() | Uses FindFirstFile() and FindNextFile() to enumerate files under the named folder |

The code is easily modified in Tester.cpp to add additional test cases.

At startup the app looks for a file named BadAssTest.txt in the same folder with the exe.  This is the configuration file that controls the testing.
This file consists of a series of lines that are independently processed.

A line starts with the file-path of a directory or folder, followed by comma delimited test Type Numbers.  A test will be made for each of those test numbers against the named file or folder.
The file-path may start with two special variables that are available only if the application is running inside an MSIX container:

* `<Root>` will be replaced by the path to the package root folder (i.e. C:\Program Files\WindowsApps\BadAssTest_3.0.0.0__x64_Sighash).
* `<WritablePackageRoot>` will be replaced by the path to the WritablePackageRoot folder (i.e. C:\Users\{UserName}\AppData\Local\Packages\BadAssTest_Sighash\LocalCache\Microsoft\Local\WritablePackageRoot).

Output is provided to the console window.  In addition, to aid in understanding interception software and mini-filter drivers, the application will write test markers into the Widnows Registry at the beginning and end of each test.
The values will be under HKCU\Software\BadAssTest.  
* `TestLine` and `ParsedLine` will be written to at the beginning of processing a line.  
* For each test on the line:
* * `TestCase` will be written with the test number at the start of that test number case.
* * The test will be run.
* * `Test Result` will be written with the result code from the test.
* When the line is finished, `Testline` will be written to with and end message.

This registry output will provide easily identifyable markers in the procmon trace deliniating those entries that are specific to a given file and test case.

## Disclaimer
This software is provided 'As-is'. Do with it as you wish.