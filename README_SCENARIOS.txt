TIVIDY INDUSTRY DEMO PATCH

Contents:
  src/main.cpp
  src/industries/HospitalAdmission.cpp
  src/industries/UniversityComplaint.cpp

Install:
1. Back up your current src/main.cpp and the two files under src/industries.
2. Extract this ZIP into the root of your TIVIDY-Prac6 repository, preserving folders and replacing those three files.
3. From the repository root, run:
     make
4. Run the demo:
     .\build\tividy.exe
   If your build outputs a different executable name, check the build folder.

The scenario files use the existing engine APIs and CMake's recursive source glob.
The notification in the university scenario is a simulated workflow step; it does not send a real email.
