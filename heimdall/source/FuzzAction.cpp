/* Copyright (c) 2010-2014 Benjamin Dobell, Glass Echidna
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.*/

// C Standard Library
#include <stdio.h>

// Heimdall
#include "Arguments.h"
#include "BridgeManager.h"
#include "FuzzAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace std;
using namespace Heimdall;

const char *FuzzAction::usage = "Action: fuzz\n\
Description: Just save some raw packets into files. Name them packet1, packet2 etc.\n\
    They'll be sent to the device sequentially\n";

int FuzzAction::Execute(int argc, char **argv)
{
	FILE *packetFile;
	int nextPacket = 1;
	unsigned int packetFileSize;
	char packetFileName[MAX_PACKET_FILENAME_LEN];
	unsigned char receivedPacket[RECV_PACKET_SIZE];
	unsigned char *packetBuffer;

	Interface::SetStdoutErrors(true);

	BridgeManager *bridgeManager = new BridgeManager(true);
	bridgeManager->SetUsbLogLevel(BridgeManager::UsbLogLevel::Debug);

	if (bridgeManager->Initialise(false) != BridgeManager::kInitialiseSucceeded) {
		Interface::PrintError("Failed to initialise BridgeManager.\n");
		return (1);
	}

	snprintf(packetFileName, sizeof(packetFileName), "packet%d", nextPacket);
	while (packetFile = FileOpen(packetFileName, "rb")) {
		Interface::Print("Opened raw packet file: %s\n", packetFileName);

		FileSeek(packetFile, 0, SEEK_END);
		packetFileSize = (unsigned int) FileTell(packetFile);
		FileRewind(packetFile);

		if (packetFileSize > MAX_FILE_SIZE) {
			Interface::PrintError("Error: File too large.\n");
			FileClose(packetFile);
			return (1);
		}

		packetBuffer = new unsigned char[packetFileSize];
		(void)fread(packetBuffer, 1, packetFileSize, packetFile);
		FileClose(packetFile);

		if (!bridgeManager->SendRawPacket(packetBuffer, packetFileSize)) {
			Interface::PrintError("Error: Failed to send raw packet.\n");
			delete [] packetBuffer;
			return (1);
		}

		delete [] packetBuffer;

		if (!bridgeManager->ReceiveRawPacket(receivedPacket, sizeof(receivedPacket))) {
			Interface::PrintError("Error: Failed to receive raw packet.\n");
		}

		nextPacket++;
		snprintf(packetFileName, sizeof(packetFileName), "packet%d", nextPacket);

	}

	Interface::Print("Packet file %s doesn't exist, handled %d packets\n", packetFileName, nextPacket - 1);

	bool success = true;

	if (!bridgeManager->EndSession(true)) {
		success = false;
	}

	delete bridgeManager;
	
	return (success ? 0 : 1);
}