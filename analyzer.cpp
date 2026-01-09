#include "analyzer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <vector>

using namespace std;

// Internal storage
unordered_map<string, long long> zoneCounts;
unordered_map<string, long long> zoneHourCounts;

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    ifstream file(csvPath);

    if (!file.is_open()) {
        cout << "File cannot be opened" << endl;
        return;
    }
    
    string line;

    // Skipping the header
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string field;
        vector<string> fields;

        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }

        // We expect exactly 6 columns
        if (fields.size() != 6) {
            continue;
        }

        string pickupZone = fields[1];
        string dateTime   = fields[3];

        // Find space between date and time
        int spacePos = dateTime.find(' ');
        if (spacePos == -1) {
            continue;
        }

        // Take hour (HH)
        string hourStr = dateTime.substr(spacePos + 1, 2);

        int hour = stoi(hourStr);

        if (hour > 23) {
            continue;
        }

        // Counting the zone
        zoneCounts[pickupZone]++;

        // Counting the zone + hour
        string key = pickupZone + "#" + to_string(hour);
        zoneHourCounts[key]++;
    }

    file.close();
}

// Sorting zones
bool compareZones(ZoneCount a, ZoneCount s) {
    if (a.count == s.count) {
        return a.zone < s.zone;
    }
    return a.count > s.count;
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> output;

    for (auto it = zoneCounts.begin(); it != zoneCounts.end(); ++it) {
        ZoneCount x;
        x.zone = it->first;
        x.count = it->second;
        output.push_back(x);
    }

    sort(output.begin(), output.end(), compareZones);

    if (output.size() > k) {
        output.resize(k);
    }

    return output;
}

// Sorting slots 
bool compareSlots(SlotCount a, SlotCount s) {
    if (a.count == s.count) {
        if (a.zone == s.zone) {
        return a.hour < s.hour;
    }
        return a.zone < s.zone;
    }
    return a.count > s.count;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> output;

    for (auto it = zoneHourCounts.begin(); it != zoneHourCounts.end(); ++it) {
        string key = it->first;
        long long cnt = it->second;

        int spacePos = key.find('#');
        if (spacePos == -1) {
            continue;
        }

        SlotCount s;
        s.zone = key.substr(0, spacePos);
        s.hour = stoi(key.substr(spacePos + 1));
        s.count = cnt;

        output.push_back(s);
    }

    sort(output.begin(), output.end(), compareSlots);

    if (output.size() > k) {
        output.resize(k);
    }

    return output;
}

