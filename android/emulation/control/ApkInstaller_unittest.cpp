// Copyright (C) 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "android/emulation/control/ApkInstaller.h"

#include "android/base/Compiler.h"
#include "android/base/system/System.h"
#include "android/base/StringView.h"
#include "android/base/testing/TestSystem.h"

#include <gtest/gtest.h>

#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

using android::base::String;
using android::base::System;
using android::base::TestSystem;
using android::base::TestTempDir;
using android::emulation::ApkInstaller;

using std::ofstream;

TEST(ApkInstaller, parseOutputForFailureNoFile) {
    String errorString;

    EXPECT_FALSE(ApkInstaller::parseOutputForFailure("i-dont-exist.txt",
                                                     &errorString));
    EXPECT_STREQ(errorString.c_str(), ApkInstaller::kDefaultErrorString);
}

TEST(ApkInstaller, parseOutputForFailureBadOutput) {
    TestSystem system("/progdir", System::kProgramBitness, "/homedir",
                      "/appdir");
    TestTempDir* dir = system.getTempRoot();
    String outputFile = dir->makeSubPath("output.txt");

    ofstream ofs(outputFile.c_str());
    EXPECT_TRUE(ofs.is_open());
    ofs << "I'm incorrectly formatted!\n"
           "Failure....\n";
    ofs.close();

    String errorString;
    EXPECT_FALSE(ApkInstaller::parseOutputForFailure(outputFile, &errorString));
    EXPECT_STREQ(errorString.c_str(), ApkInstaller::kDefaultErrorString);
}

TEST(ApkInstaller, parseOutputForFailureInstallFailed) {
    TestSystem system("/progdir", System::kProgramBitness, "/homedir",
                      "/appdir");
    TestTempDir* dir = system.getTempRoot();
    String outputFile = dir->makeSubPath("output.txt");

    ofstream ofs(outputFile.c_str());
    EXPECT_TRUE(ofs.is_open());
    ofs << "542 KB/s (25370 bytes in 0.045s)\n"
           "        pkg: /data/local/tmp/an-apk-that-failed.apk\n"
           "Failure [INSTALL_FAILED_OLDER_SDK]\n";
    ofs.close();

    String errorString;
    EXPECT_FALSE(ApkInstaller::parseOutputForFailure(outputFile, &errorString));
    EXPECT_STREQ(errorString.c_str(), "INSTALL_FAILED_OLDER_SDK");
}

TEST(ApkInstaller, parseOutputForFailureInstallSuccess) {
    TestSystem system("/progdir", System::kProgramBitness, "/homedir",
                      "/appdir");
    TestTempDir* dir = system.getTempRoot();
    String outputFile = dir->makeSubPath("output.txt");

    ofstream ofs(outputFile.c_str());
    EXPECT_TRUE(ofs.is_open());
    ofs << "5196 KB/s (1097196 bytes in 0.206s)\n"
           "         pkg: /data/local/tmp/an-apk-that-installed.apk\n"
           "Success\n";
    ofs.close();

    String errorString;
    EXPECT_TRUE(ApkInstaller::parseOutputForFailure(outputFile, &errorString));
}
