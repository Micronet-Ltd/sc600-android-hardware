/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hardware/google/pixelstats/1.0/IPixelStats.h>

#include <metricslogger/metrics_logger.h>
#include <utils/StrongPointer.h>

#include <getopt.h>
#include <iostream>

using hardware::google::pixelstats::V1_0::IPixelStats;

void expect_message(int32_t action) {
    std::cout << "expect sysui_multi_action: [757," << action << ",\n";
}

void show_help() {
    std::cout << "Pixelstats HAL client\n";
    std::cout << " arguments:\n";
    std::cout << " -c or --UsbConnectorConnected\n";
    std::cout << " -C or --UsbConnectorDisconnected\n";
    std::cout << " -a or --UsbAudioConnected\n";
    std::cout << " -A or --UsbAudioDisconnected\n";
    std::cout << " -S or --SpeakerImpedance\n";
    std::cout << " -f or --HardwareFailed\n";
    std::cout << " -y or --ChargeCycles\n";
    std::cout << " -n or --BatteryHealthSnapshot\n";
    std::cout << " -i or --SlowIo\n";
    std::cout << " -s or --BatteryCausedShutdown\n";
    std::cout << "\n\n you can use multiple arguments to trigger multiple events.\n";
}

int main(int argc, char* argv[]) {
    using namespace hardware::google::pixelstats::V1_0;

    android::sp<IPixelStats> client = IPixelStats::tryGetService();
    if (!client) {
        std::cerr << "No PixelStats HAL";
        return 1;
    }

    static struct option opts[] = {
        {"UsbConnectorConnected", no_argument, 0, 'c'},
        {"UsbConnectorDisconnected", no_argument, 0, 'C'},
        {"UsbAudioConnected", no_argument, 0, 'a'},
        {"UsbAudioDisconnected", no_argument, 0, 'A'},
        {"SpeakerImpedance", no_argument, 0, 'S'},
        {"HardwareFailed", no_argument, 0, 'f'},
        {"ChargeCycles", no_argument, 0, 'y'},
        {"BatteryHealthSnapshot",  no_argument, 0, 'n'},
        {"SlowIo",  no_argument, 0, 'i'},
        {"BatteryCausedShutdown", no_argument, 0, 's'},
    };

    int c;
    int hal_calls = 0;
    while ((c = getopt_long(argc, argv, "cCaASfynis", opts, nullptr)) != -1) {
        switch (c) {
            case 'c':
                client->reportUsbConnectorConnected();
                expect_message(android::metricslogger::ACTION_USB_CONNECTOR_CONNECTED);
                ++hal_calls;
                break;
            case 'C':
                client->reportUsbConnectorDisconnected(60);
                expect_message(android::metricslogger::ACTION_USB_CONNECTOR_DISCONNECTED);
                ++hal_calls;
                break;
            case 'a':
                client->reportUsbAudioConnected(0x5555, 0x1212);
                expect_message(android::metricslogger::ACTION_USB_AUDIO_CONNECTED);
                ++hal_calls;
                break;
            case 'A':
                client->reportUsbAudioDisconnected(0x5555, 0x1212, 500);
                expect_message(android::metricslogger::ACTION_USB_AUDIO_DISCONNECTED);
                ++hal_calls;
                break;
            case 'S':
                client->reportSpeakerImpedance(10, 1234);
                expect_message(android::metricslogger::ACTION_SPEAKER_IMPEDANCE);
                ++hal_calls;
                break;
            case 'f':
                client->reportHardwareFailed(IPixelStats::HardwareType::UNKNOWN, 5,
                                             IPixelStats::HardwareErrorCode::COMPLETE);
                expect_message(android::metricslogger::ACTION_HARDWARE_FAILED);
                ++hal_calls;
                break;
            case 'y':
                client->reportChargeCycles("1,2,3,4,5,6,7,8");
                expect_message(android::metricslogger::ACTION_BATTERY_CHARGE_CYCLES);
                ++hal_calls;
                break;
            case 'n':
                IPixelStats::BatteryHealthSnapshotArgs args;
                args.temperatureDeciC = 3000;
                args.voltageMicroV = 1;
                args.currentMicroA = 2;
                args.openCircuitVoltageMicroV = 3;
                args.resistanceMicroOhm = 5;
                args.levelPercent = 101;
                client->reportBatteryHealthSnapshot(args);
                expect_message(android::metricslogger::ACTION_BATTERY_HEALTH);
                ++hal_calls;
                break;
            case 'i':
                client->reportSlowIo(IPixelStats::IoOperation::UNKNOWN, 5);
                expect_message(android::metricslogger::ACTION_SLOW_IO);
                ++hal_calls;
                break;
            case 's':
                client->reportBatteryCausedShutdown(3);
                expect_message(android::metricslogger::ACTION_BATTERY_CAUSED_SHUTDOWN);
                ++hal_calls;
                break;
        }
    }

    if (hal_calls == 0) {
        show_help();
    } else {
        std::cout << hal_calls << " HAL methods called.\n";
        std::cout << "try: logcat -b events | grep \"sysui_multi_action: \\\\[757\"\n";
    }
    return 0;
}
