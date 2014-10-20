#include <map>
#include <string>
#include <vector>

// Generic Point class, for heatmaps. Just to be thorough.
class Point
{
public:
    Point(void) : x(0.f), y(0.f), z(0.f) {};
    Point(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
    float x;
    float y;
    float z;
};

// You'll put together a LogEvent whenever you want to log something.
class LogEvent
{
    friend class Logger;
public:
    LogEvent() : m_useArea(false), m_useEventID(false), m_useValue(false), m_useLocation(false) {};
    void SetArea(std::string area);
    void SetEventID(std::string eventID);
    void SetValue(float value);
    void SetLocation(Point location);
private:
    std::string GetString(std::string userID, std::string sessionID, std::string build);

    bool m_useArea;
    bool m_useEventID;
    bool m_useValue;
    bool m_useLocation;

    std::string m_area;
    std::string m_eventID;
    float m_value;
    Point m_location;
};

// The main class for logging and sending events.
class Logger
{
public:
    Logger(void);
    void AddLogEvent(LogEvent ev);
    void SubmitLogEvents(void);
    void LoadHeatmap(std::string area, std::string eventID);
    bool GetHeatmap(std::string area, std::vector<std::pair<Point, int>>& points);

	// You'll get your game's unique public key from GameAnalytics.
	void SetGameKey(const std::string &gameKey);
    // You'll also get an API key from GameAnalytics.
	void SetApiKey(const std::string &apiKey);
    // The build version you're on.
	void SetBuild(const std::string &build);
    // You're on secret key.
	void SetSecretKey(const std::string &secretKey);

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
    std::map<std::string, std::vector<std::pair<Point, int>>> m_heatmaps;
};
