
#pragma once

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <map>
#include <string>
#include <vector>

#include "vector3.h"

// You'll put together an AnalyticsEvent whenever you want to log something.
class AnalyticsEvent
{
    friend class GameAnalytics;
public:
    AnalyticsEvent() : m_useArea(false), m_useEventID(false), m_useValue(false), m_useLocation(false) {};
	AnalyticsEvent(const std::string& evID) : m_useArea(false), m_useEventID(true), m_useValue(false), m_useLocation(false), m_eventID(evID) {};
	AnalyticsEvent(const std::string& evID, const vector3d& location) : m_useArea(false), m_useEventID(true), m_useValue(false), m_useLocation(true), m_eventID(evID), m_location(location) {};

    void SetArea(std::string area);
    void SetEventID(std::string eventID);
    void SetValue(float value);
    void SetLocation(const vector3d& location);
private:
    std::string GetString(std::string userID, std::string sessionID, std::string build);

    bool m_useArea;
    bool m_useEventID;
    bool m_useValue;
    bool m_useLocation;

    std::string m_area;
    std::string m_eventID;
    float m_value;
    vector3d m_location;
};

// The main class for logging and sending events.
class Analytics
{
public:
    Analytics(void) {}
	virtual ~Analytics(void) {}
    virtual void AddLogEvent(AnalyticsEvent ev) {}
    virtual void SubmitLogEvents(void) {}
    virtual void LoadHeatmap(std::string area, std::string eventID) {}
    virtual bool GetHeatmap(std::string area, std::vector<std::pair<vector3d, int>>& points) { return false; }

	// You'll get your game's unique public key from GameAnalytics.
	virtual void SetGameKey(const std::string &gameKey) {}
    // You'll also get an API key from GameAnalytics.
	virtual void SetApiKey(const std::string &apiKey) {}
    // The build version you're on.
	virtual void SetBuild(const std::string &build) {}
    // You're on secret key.
	virtual void SetSecretKey(const std::string &secretKey) {}
};

#ifdef USE_GAME_ANALYTICS_LOGGING
// The main class for logging and sending events.
class GameAnalytics : public Analytics
{
public:
    GameAnalytics(void);
	virtual ~GameAnalytics(void) {}
    virtual void AddLogEvent(AnalyticsEvent ev);
    virtual void SubmitLogEvents(void);
    virtual void LoadHeatmap(std::string area, std::string eventID);
    virtual bool GetHeatmap(std::string area, std::vector<std::pair<vector3d, int>>& points);

	// You'll get your game's unique public key from GameAnalytics.
	virtual void SetGameKey(const std::string &gameKey);
    // You'll also get an API key from GameAnalytics.
	virtual void SetApiKey(const std::string &apiKey);
    // The build version you're on.
	virtual void SetBuild(const std::string &build);
    // You're on secret key.
	virtual void SetSecretKey(const std::string &secretKey);

private:
    void SetUserID(void);
    void SetSessionID(void);

    // Yes I know this is a lot of strings. DEAL.
    std::string m_apiVersion;
    std::string m_gameKey;
    std::string m_apiKey;
	std::string m_secretKey;
    std::string m_build;
    std::string m_gaEvents;
    std::string m_userID;
    std::string m_sessionID;

    // This makes PERFECT SENSE.
    std::map<std::string, std::vector<std::pair<vector3d, int>>> m_heatmaps;
};
#endif // USE_GAME_ANALYTICS_LOGGING

#endif // _LOGGER_H_
