#include "Logger.h"
#include "SYSTEM.h"

#include <sstream>

#include "curl/curl.h" // libcurl, for sending http requests
#include "MD5.h"  // MD5 hashing algorithm
#include "json\json.h" // json parser

// Each Set function also specifies that the variable is in use.
// That way it won't be added to the event string.
// There might be a more elegant way to do this that I haven't thought of.

void LogEvent::SetArea(std::string area)
{
    m_area = area;
    m_useArea = true;
}

void LogEvent::SetEventID(std::string eventID)
{
    m_eventID = eventID;
    m_useEventID = true;
}

void LogEvent::SetValue(float value)
{
    m_value = value;
    m_useValue = true;
}

void LogEvent::SetLocation(Point location)
{
    m_location = location;
    m_useLocation = true;
}

// Gets the string version of the event, ready to send.
// It might be nicer to use a json generator, but the format is simple enough.
// For reference on what this should like, go here: http://support.gameanalytics.com/entries/22613463-Design-event-structure
std::string LogEvent::GetString(std::string userID, std::string sessionID, std::string build)
{
    std::stringstream ss;
    ss << "{";
    ss << "\"user_id\": ";
    ss << "\"" << userID << "\"";
    ss << ", ";
    ss << "\"session_id\": ";
    ss << "\"" << sessionID << "\"";
    ss << ", ";
    ss << "\"build\": ";
    ss << "\"" << build << "\"";
    if (m_useArea)
    {
        ss << ", ";
        ss << "\"area\": ";
        ss << "\"" << m_area << "\"";
    }
    if (m_useEventID)
    {
        ss << ", ";
        ss << "\"event_id\": ";
        ss << "\"" << m_eventID << "\"";
    }
    if (m_useValue)
    {
        ss << ", ";
        ss << "\"value\": ";
        ss << m_value;
    }
    if (m_useLocation)
    {
        ss << ", ";
        ss << "\"x\": ";
        ss << m_location.x;
        ss << ", ";
        ss << "\"y\": ";
        ss << m_location.y;
        ss << ", ";
        ss << "\"z\": ";
        ss << m_location.z;
    }
    ss << "}";
    return ss.str();
}

Logger::Logger(void)
{
    // Initialize libcurl.
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);

    // Check the error flag.
    if (res != CURLE_OK)
    {
        // This is how I roll with error messages.
        std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
    }

    // Currently the API's version is 1. Leave it like this for now.
    m_apiVersion = "1";

    // This will be a concatenation of all events into one string,
    //   enclosed by brackets.
    m_gaEvents = "[";
}

// Although it's necessary to include a user ID and session ID in an event,
//   there's no regulation on what that entails. I use the following two
//   functions to ensure that the user ID is unique to each machine the
//   game is played on, and the session ID is unique every time.
// More info here: http://support.gameanalytics.com/entries/23054568-Generating-unique-user-identifiers

// This hashes the user's MAC address to create an ID unique to that machine.
void Logger::SetUserID(void)
{
    // GetUniqueUserID uses Windows-specific code, so it's separated
    //   into a separate file.
    m_userID = SYSTEM::GetUniqueUserID();
}

// Sets the session ID with a GUID (Globally Unique Identifier).
void Logger::SetSessionID(void)
{
    // GetGUID uses Windows-specific code, so it's separated
    //   into a separate file.
    m_sessionID = SYSTEM::GetGUID();
}

// Adds an event to the string of all events.
// This allows you to decide when to send events to the server,
//   as opposed to sending one every time an event is logged.
void Logger::AddLogEvent(LogEvent ev)
{
    // If this isn't the first event...
    if (m_gaEvents.back() != '[')
    {
        // ...add a comma to separate this one from the last.
        m_gaEvents += ", ";
    }
    // Add the event to the string.
    m_gaEvents += ev.GetString(m_userID, m_sessionID, m_build);
}

// Submits the events string to the server.
// This is the meat of the whole operation.
// (I think I mixed up two phrases there...)
void Logger::SubmitLogEvents(void)
{
    // New CURL object.
    CURL * curl = curl_easy_init();
    if (curl)
    {
        // Close the events string with a bracket.
        m_gaEvents += "]";

        // ---------------------------------- //
        // ------PARTIAL IMPLEMENTATION------ //
        // ---------------------------------- //
        // There are multiple categories of events, but I only use "design" so far.
        // To use different categories you would likely have to rearrange this system,
        //   perhaps by having different events strings for different categories.
        const std::string category = "design";

        // The URL to send events to
        const std::string url = "http://api.gameanalytics.com/" + m_apiVersion + "/" + m_gameKey + "/" + category;

        // Combine your events string and secret key for the header.
        const std::string header = m_gaEvents + m_secretKey;
        // Then hash that, and give it the label "Authorization:".
        const std::string auth = "Authorization:" + md5(header);

        // Create a struct of headers.
        struct curl_slist *chunk = NULL;
        // Add the auth header to the struct.
        chunk = curl_slist_append(chunk, auth.c_str());

        // Set the URL of the request.
        CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, url);

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // This is how I roll with error messages.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Set the POST data (the string of events).
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_gaEvents.c_str());

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // This is how I roll with error messages.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Set the headers (hashed secret key + data).
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // This is how I roll with error messages.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Set request mode to POST.
        res = curl_easy_setopt(curl, CURLOPT_POST, 1);

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // This is how I roll with error messages.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Send the request!
        res = curl_easy_perform(curl);

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // This is how I roll with error messages.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup your CURL object.
        curl_easy_cleanup(curl);
        // Free the headers.
        curl_slist_free_all(chunk);
    }

    // Start the events string over again.
    m_gaEvents = "[";
}

// Callback to take returned data and convert it to a string.
// The last parameter is actually user-defined - it can be whatever
//   type you want it to be.
int DataToString(char *data, size_t size, size_t nmemb, std::string * result)
{
    int count = 0;
    if (result)
    {
        // Data is in a character array, so just append it to the string.
        result->append(data, size * nmemb);
        count = size * nmemb;
    }
    return count;
}

// Get heatmap data for an area, and add it to your collection of heatmaps.
void Logger::LoadHeatmap(std::string area, std::string eventID)
{
    // New CURL object.
    CURL * curl = curl_easy_init();
    if (curl)
    {
        // Request URL for getting heatmap data.
        std::string url = "http://data-api.gameanalytics.com/heatmap";
        // Specify what area the heatmap should be from, and what events it
        //   should be getting data for.
        // Note: you can ask for multiple events at the same time with |,
        //   like "Death:Enemy|Death:Fall".
        std::string requestInfo = "game_key=" + m_gameKey + "&event_ids=" + eventID + "&area=" + area;
        // Concatenate the request with the URL.
        url += "/?";
        url += requestInfo;

        // Build the header with the request and API key.
        std::string header = requestInfo + m_apiKey;
        // Hash that, and give it the label "Authorization:".
        std::string auth = "Authorization:" + md5(header);

        // Create a struct of headers.
        struct curl_slist *chunk = NULL;
        // Add the auth header to the struct.
        chunk = curl_slist_append(chunk, auth.c_str());

        // Set the URL of the request.
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set the headers (hashed API key + request).
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        // Set the callback to deal with the returned data.
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DataToString);

        std::string result;

        // Pass in a string to accept the result.
        // (This will be taken care of in the callback function.)
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        // Send the request!
        CURLcode res = curl_easy_perform(curl);

        // Check the error flag.
        if (res != CURLE_OK)
        {
            // Bummer.
            std::cout << "Curl didn't not even work. " << curl_easy_strerror(res) << std::endl;
        }

        // Get ready to receive json data.
		Json::Reader reader;
		Json::Value data;

        // Parse json data.
        if(!reader.parse(result, data))
        {
            // ---------------------- //
            // ------INCOMPLETE------ //
            // ---------------------- //
            /* deal with error */
        }

        // Data returned consists of 4 arrays: x, y, z, and value.
        // x, y, and z being coordinates of course, and value being
        //   the number of instances at that point.
        Json::Value & x = data["x"];
        Json::Value & y = data["y"];
        Json::Value & z = data["z"];
        Json::Value & value = data["value"];

        // Make a vector of pairs: points and their values.
        std::vector<std::pair<Point, int>> points;

        // Iterate through the arrays. (We're just iterating through x technically,
        //   but all arrays are the same length.)
        for (unsigned i = 0; i < x.size(); ++i)
        {
            // Add x, y, and z as a Point, and value as the second part of the pair.
            points.push_back(std::pair<Point, int>(Point(x[i].asDouble(), y[i].asDouble(), z[i].asDouble()), value[i].asInt()));
        }

        // I like to keep my heatmaps ordered by area.
        // You can organize them however makes sense to you.
        m_heatmaps.insert(std::pair<std::string, std::vector<std::pair<Point, int>>>(area, points));
        
        // Cleanup your CURL object.
        curl_easy_cleanup(curl);
        // Free the headers.
        curl_slist_free_all(chunk);
    }
}

// Gets a heatmap from a particular area. If that heatmap doesn't exist, returns false.
bool Logger::GetHeatmap(std::string area, std::vector<std::pair<Point, int>>& points)
{
    std::map<std::string, std::vector<std::pair<Point, int>>>::iterator it = m_heatmaps.find(area);
    if (it == m_heatmaps.end())
        return false;

    points = (*it).second;
    return true;
}

// You'll get your game's unique public key from GameAnalytics.
void Logger::SetGameKey(const std::string &gameKey) 
{ 
	m_gameKey = gameKey; 
}

// You'll also get an API key from GameAnalytics (for getting heatmaps).
void Logger::SetApiKey(const std::string &apiKey)
{ 
	m_apiKey = apiKey; 
}

// The build version you're on. Set this however works best for you.
void Logger::SetBuild(const std::string &build) 
{ 
	m_build = build; 
}

// You're on secret key.
void Logger::SetSecretKey(const std::string &secretKey)
{
	m_secretKey = secretKey;
}
