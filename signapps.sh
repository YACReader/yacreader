#!/bin/bash

echo "Signing apps"

codesign --force --deep --sign "Developer ID Application: LUIS ANGEL SAN MARTIN ROD (9B6KKVW3WM)" ./YACReader.app
codesign --force --deep --sign "Developer ID Application: LUIS ANGEL SAN MARTIN ROD (9B6KKVW3WM)" ./YACReaderLibrary.app
codesign --force --deep --sign "Developer ID Application: LUIS ANGEL SAN MARTIN ROD (9B6KKVW3WM)" ./YACReaderLibraryServer.app
