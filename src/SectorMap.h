// Copyright © 2008-2024 Pioneer Developers
// See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORMAP_H
#define _SECTORMAP_H

#include "Color.h"
#include "DeleteEmitter.h"
#include "Input.h"
#include "RefCounted.h"
#include "galaxy/Factions.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "SectorMapContext.h"

namespace Graphics {
	class RenderTarget;
}

/**
 * @class SectorMap
 * @brief The primary class responsible for rendering and interacting with
 *        the in-game sector map. Supports 3D draws, embedded draws (ImGui),
 *        labeling, navigation, and more.
 *
 * SectorMap displays and manages the star systems in a 3D sector-based layout.
 * Players can rotate, zoom, and explore the galaxy's local neighborhood. This
 * class also manages caching of sectors, labeling logic, and a variety of draw
 * operations (lines, billboards, spheres).
 */
class SectorMap : public DeleteEmitter {
public:
	/**
	 * @brief Construct a SectorMap with a given context, typically specifying 
	 *        the player's current location, FOV settings, etc.
	 * @param context  The SectorMapContext containing config & environment data.
	 */
	explicit SectorMap(SectorMapContext &&context);

	/**
	 * @brief Construct a SectorMap from saved JSON data and an initial context.
	 * @param jsonObj  A JSON object containing saved state data.
	 * @param context  The SectorMapContext used for finalizing configuration.
	 */
	SectorMap(const Json &jsonObj, SectorMapContext &&context);

	/**
	 * @brief Destructor.
	 * Handles cleanup of any GPU resources, allocated materials, etc.
	 */
	~SectorMap();

	/**
	 * @brief Update the map logic based on elapsed time.
	 * @param frameTime  The time in seconds since last frame (delta time).
	 */
	void Update(float frameTime);

	/**
	 * @brief Draw all 3D elements of the sector map (stars, lines, etc.).
	 */
	void Draw3D();

	/**
	 * @brief Draw system labels (and possibly faction labels) using ImGui overlay.
	 * @param interactive  If true, labels can respond to input (mouse hover/click).
	 * @param imagePos     The top-left position for label rendering (for offset).
	 */
	void DrawLabels(bool interactive, const ImVec2 &imagePos = { 0.0, 0.0 });

	/**
	 * @brief Draw an embedded sector map (for small sub-windows, e.g. in UI).
	 */
	void DrawEmbed();

	/**
	 * @brief Get the current position vector (in sector coordinates).
	 * @return The current position in 3D space, as a vector3f.
	 */
	vector3f GetPosition() const { return m_pos; }

	/**
	 * @brief Translate a SystemPath to local 3D coordinates for the star in the sector.
	 * @param path The star system path to find coordinates for.
	 * @return The 3D position of the system in the current viewpoint's coordinate system.
	 */
	vector3f GetSystemPosition(const SystemPath &path);

	/**
	 * @brief Retrieve the current map zoom level.
	 * @return Zoom level as a double (larger = more zoom out).
	 */
	double GetZoomLevel() const;

	/**
	 * @brief Retrieve (or load) the sector data from the sector cache.
	 * @param loc The path specifying which sector to retrieve.
	 * @return RefCountedPtr to the requested Sector object, or nullptr if invalid.
	 */
	RefCountedPtr<Sector> GetCached(const SystemPath &loc) { return m_sectorCache->GetCached(loc); }

	/**
	 * @brief Check if the map is centered on the given system path.
	 * @param path The system path to check.
	 * @return True if the map is currently centered on this system, false otherwise.
	 */
	bool IsCenteredOn(const SystemPath &path);

	/**
	 * @brief Check if the map is being manually moved (as opposed to auto-follow).
	 * @return True if the user is manually controlling the position, false otherwise.
	 */
	bool IsManualMove() { return m_manualMove; }

	/**
	 * @brief Find the nearest star system to a given position.
	 * @param pos The local 3D position to test against.
	 * @return The SystemPath for the closest system.
	 */
	SystemPath NearestSystemToPos(const vector3f &pos);

	/**
	 * @brief Get the current 4x4 transform matrix that describes point-of-view for draws.
	 * @return The matrix4x4f for the camera's viewpoint.
	 */
	matrix4x4f PointOfView();

	/**
	 * @brief Retrieve the SectorMapContext used for configuration, camera, etc.
	 * @return Reference to the SectorMapContext.
	 */
	SectorMapContext &GetContext() { return m_context; }

	/**
	 * @brief Enable or disable the drawing of uninhabited star labels.
	 * @param value True to draw, false to omit them.
	 */
	void SetDrawUninhabitedLabels(bool value) { m_drawUninhabitedLabels = value; }

	/**
	 * @brief Enable or disable vertical lines that connect star systems.
	 * @param value True to draw, false to hide.
	 */
	void SetDrawVerticalLines(bool value) { m_drawVerticalLines = value; }

	/**
	 * @brief Specify whether a given faction's stars are visible on the map.
	 * @param faction Pointer to the faction to show/hide.
	 * @param visible True if visible, false to hide them.
	 */
	void SetFactionVisible(const Faction *faction, bool visible);

	/**
	 * @brief Enable or disable zoom mode (e.g., user controlling map zoom).
	 * @param enable True if zoom mode is on.
	 */
	void SetZoomMode(bool enable);

	/**
	 * @brief Enable or disable rotate mode (user controlling rotation).
	 * @param enable True if rotate mode is on.
	 */
	void SetRotateMode(bool enable);

	/**
	 * @brief Configure label drawing parameters (fonts, sizes, colors).
	 * @param fontName  The name of the font to use for star labels.
	 * @param fontSize  Size in px or pt (implementation-specific).
	 * @param gap       Spacing between text lines / label edges.
	 * @param highlight The highlight color used for label backgrounds, etc.
	 * @param shade     The shading color used for text or overlays.
	 */
	void SetLabelParams(std::string fontName, int fontSize, float gap, Color highlight, Color shade);

	/**
	 * @brief Hide or show all labels in the sector map.
	 * @param hideLabels True to hide, false to show.
	 */
	void SetLabelsVisibility(bool hideLabels) { m_hideLabels = hideLabels; }

	/**
	 * @brief Moves the camera to a specific sector.
	 * @param path The SystemPath specifying the desired sector.
	 */
	void GotoSector(const SystemPath &path);

	/**
	 * @brief Moves the camera to a specific system (inside a sector).
	 * @param path The SystemPath specifying the desired system.
	 */
	void GotoSystem(const SystemPath &path);

	/**
	 * @brief Zoom in one step.
	 */
	void ZoomIn();

	/**
	 * @brief Zoom out one step.
	 */
	void ZoomOut();

	/**
	 * @brief Reset the map view to default rotation, zoom, and position.
	 */
	void ResetView();

	/**
	 * @brief Create an offscreen render target for the map (e.g., for embedding).
	 */
	void CreateRenderTarget();

	/**
	 * @brief Adjust the map's rendering area (width, height).
	 * @param size  The new dimensions in pixels.
	 */
	void SetSize(vector2d size);

	/**
	 * @brief Serialize the current map state into a JSON object.
	 * @param jsonObj The JSON object to fill with map state data.
	 */
	void SaveToJson(Json &jsonObj);

	/**
	 * @brief Create a star billboard sprite in the 3D map.
	 * @param modelview The model-view transform for positioning.
	 * @param pos       The star's position (local coords).
	 * @param col       The color to draw the billboard.
	 * @param size      The billboard size (scale).
	 */
	void AddStarBillboard(const matrix4x4f &modelview, const vector3f &pos, const Color &col, float size);

	/**
	 * @brief Add a sphere to be drawn on the map.
	 * @param trans The sphere's transform matrix.
	 * @param color Optional color override (default White).
	 */
	void AddSphere(const matrix4x4f &trans, const Color &color = Color::WHITE);

	/**
	 * @brief Add a single line vertex to be drawn.
	 * @param v The position of the line vertex.
	 * @param c The color of this line segment.
	 *
	 * Note: Lines are not auto-cleared every frame. Clear them manually if needed.
	 */
	void AddLineVert(const vector3f &v, const Color &c);

	/**
	 * @brief Clear all line vertices previously added.
	 */
	void ClearLineVerts();

	/**
	 * @brief Find star systems (by name) in the local region.
	 * @param pattern Substring or pattern to match (case or partial).
	 * @return A list of matching SystemPaths.
	 */
	std::vector<SystemPath> GetNearbyStarSystemsByName(std::string pattern);

	/**
	 * @brief Get the set of currently visible factions in the map.
	 * @return A const reference to the set of visible factions.
	 */
	const std::set<const Faction *> &GetVisibleFactions() { return m_visibleFactions; }

	/**
	 * @brief Get the set of currently hidden factions in the map.
	 * @return A const reference to the set of hidden factions.
	 */
	const std::set<const Faction *> &GetHiddenFactions() { return m_hiddenFactions; }

	/**
	 * @struct InputBinding
	 * @brief Manages the user input actions and axes for the sector map (move, zoom, warp, etc.).
	 */
	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		Action *mapToggleSelectionFollowView;
		Action *mapWarpToCurrent;
		Action *mapWarpToSelected;
		Action *mapViewReset;

		Axis *mapViewMoveForward;
		Axis *mapViewMoveLeft;
		Axis *mapViewMoveUp;
		Axis *mapViewYaw;
		Axis *mapViewPitch;
		Axis *mapViewZoom;

		/**
		 * @brief Register the input bindings for the sector map.
		 */
		void RegisterBindings() override;
	} InputBindings;

private:
	/**
	 * @brief Initialize default values for variables (called by constructors).
	 */
	void InitDefaults();

	/**
	 * @brief Internal setup function (e.g., loads fonts, materials).
	 */
	void InitObject();

	/**
	 * @brief Draws the 'near' sectors (close to the player's current sector).
	 * @param modelview The transform matrix used for 3D rendering.
	 */
	void DrawNearSectors(const matrix4x4f &modelview);

	/**
	 * @brief Draw a single sector (by coordinates) in 'near' mode.
	 * @param sx,sy,sz The sector coordinates.
	 * @param trans    The current transformation matrix.
	 */
	void DrawNearSector(const int sx, const int sy, const int sz, const matrix4x4f &trans);

	/**
	 * @brief Internal function to draw system labels.
	 */
	void DrawLabelsInternal(bool interactive, const ImVec2 &imagePos = { 0.0, 0.0 });

	/**
	 * @brief Helper to place labels for a Sector's star systems.
	 * @param sec        The sector reference.
	 * @param origin     The local origin position in 3D space.
	 * @param drawRadius The radius (in sectors) for which to place labels.
	 */
	void PutSystemLabels(RefCountedPtr<Sector> sec, const vector3f &origin, int drawRadius);

	/**
	 * @brief Draw a single system label (optionally with a shadow).
	 * @param sys   The system data to label.
	 * @param shadow If true, draw a shadow text behind it.
	 */
	void PutSystemLabel(const Sector::System &sys, bool shadow);

	/**
	 * @brief Draw the 'far' sectors (those beyond the near radius).
	 * @param modelview The transform matrix used for 3D rendering.
	 */
	void DrawFarSectors(const matrix4x4f &modelview);

	/**
	 * @brief Prepares arrays for far sector rendering (points & colors).
	 * @param sec     The sector reference.
	 * @param origin  The local origin position.
	 * @param points  Output array of positions.
	 * @param colors  Output array of colors.
	 */
	void BuildFarSector(RefCountedPtr<Sector> sec, const vector3f &origin,
						std::vector<vector3f> &points, std::vector<Color> &colors);

	/**
	 * @brief Places faction labels if needed at the given sector position.
	 * @param secPos The sector's 3D position relative to the map center.
	 */
	void PutFactionLabels(const vector3f &secPos);

	/**
	 * @brief Handle a user clicking on a star label in the map.
	 * @param path The SystemPath for the clicked star.
	 */
	void OnClickLabel(const SystemPath &path);

	/**
	 * @brief Attempt to shrink or trim the cache if usage is too large or stale.
	 */
	void ShrinkCache();

	/**
	 * @brief Marks a system as selected, storing its path.
	 * @param path The system path to select.
	 */
	void SetSelected(const SystemPath &path);

	/**
	 * @brief Callback for mouse-wheel input, zooms in or out.
	 * @param up True if wheel scrolled up, false if down.
	 */
	void MouseWheel(bool up);

	// -------------------
	// Member variables
	// -------------------

	/// The overarching context (camera, config, etc.) used by this map.
	SectorMapContext m_context;

	/// Current map position in 3D (sector-based coordinate system).
	vector3f m_pos;

	/// The position we're moving toward if a smooth transition is in progress.
	vector3f m_posMovingTo;

	/// Default rotation about X-axis, used in ResetView.
	float m_rotXDefault;
	/// Default rotation about Z-axis, used in ResetView.
	float m_rotZDefault;
	/// Default zoom level, used in ResetView.
	float m_zoomDefault;

	/// Current rotation around X-axis.
	float m_rotX;
	/// Current rotation around Z-axis.
	float m_rotZ;

	/// Target rotation for smooth transitions (X).
	float m_rotXMovingTo;
	/// Target rotation for smooth transitions (Z).
	float m_rotZMovingTo;

	/// Current zoom level.
	float m_zoom;
	/// Zoom clamped to [min, max] range.
	float m_zoomClamped;
	/// Target zoom level for smooth transitions.
	float m_zoomMovingTo;

	bool m_rotateWithMouseButton = false; ///< Flag to rotate map if mouse button is held.
	bool m_rotateView = false;            ///< True if user is actively rotating view.
	bool m_zoomView = false;              ///< True if user is actively zooming view.
	bool m_manualMove = false;            ///< True if user input is overriding the camera.

	bool m_drawUninhabitedLabels = false; ///< True to draw labels for uninhabited systems.
	bool m_drawVerticalLines = false;     ///< True to draw vertical lines from stars to grid.

	std::set<const Faction *> m_visibleFactions; ///< Factions explicitly shown.
	std::set<const Faction *> m_hiddenFactions;  ///< Factions explicitly hidden.

	Uint8 m_detailBoxVisible;

	sigc::connection m_onToggleSelectionFollowView;
	sigc::connection m_onWarpToSelected;
	sigc::connection m_onViewReset;

	/// Pointer to the sector cache that manages star system data in memory.
	RefCountedPtr<SectorCache::Slave> m_sectorCache;

	std::vector<vector3f> m_farstars;      ///< Positions of stars in far sectors.
	std::vector<Color> m_farstarsColor;    ///< Colors associated with each far star.

	vector3f m_secPosFar;                 ///< For far sector calculations, track center pos.
	int m_radiusFar;                       ///< Range of far sectors from current center.
	bool m_toggledFaction;                 ///< Internal flag if faction toggles changed.

	int m_cacheXMin, m_cacheXMax;         ///< Cache boundary in X dimension.
	int m_cacheYMin, m_cacheYMax;         ///< Cache boundary in Y dimension.
	int m_cacheZMin, m_cacheZMax;         ///< Cache boundary in Z dimension.

	/// ImGui draw list for custom UI / label drawing, etc.
	std::unique_ptr<ImDrawList> m_drawList;
	/// Offscreen render target for embedded views.
	std::unique_ptr<Graphics::RenderTarget> m_renderTarget;
	/// Current map size for the ImGui viewport or rendering area.
	ImVec2 m_size;
	bool m_needsResize;

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	std::unique_ptr<Graphics::VertexArray> m_secLineVerts;
	std::unique_ptr<Graphics::VertexArray> m_starVerts;
	std::unique_ptr<Graphics::VertexArray> m_customLineVerts;
	bool m_updateCustomLines = false; ///< Flag to indicate custom line data changed.

	RefCountedPtr<Graphics::Material> m_starMaterial;
	RefCountedPtr<Graphics::Material> m_fresnelMat;
	RefCountedPtr<Graphics::Material> m_lineMat;
	RefCountedPtr<Graphics::Material> m_farStarsMat;

	Graphics::Drawables::Lines m_lines;        ///< Basic lines for the map.
	Graphics::Drawables::Lines m_customlines;  ///< User-defined custom lines.
	Graphics::Drawables::Lines m_sectorlines;  ///< Grid lines or sector boundary lines.
	Graphics::Drawables::Points m_farstarsPoints;

	/**
	 * @struct SphereParam
	 * @brief Parameters for drawing a sphere with color & transform.
	 */
	struct SphereParam {
		matrix4x4f trans;
		Color color;
	};
	std::vector<SphereParam> m_sphereParams; ///< Queued sphere draws.
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_sphere; ///< Sphere mesh for 3D draws.

	/**
	 * @class Label
	 * @brief Base class for different label types (factions, stars, etc.).
	 * Implementation details are internal to the .cpp.
	 */
	class Label;

	/**
	 * @class StarLabel
	 * @brief Specialized label to display star system names.
	 */
	class StarLabel;

	/**
	 * @class FactionLabel
	 * @brief Specialized label to display faction info.
	 */
	class FactionLabel;

	/**
	 * @struct Labels
	 * @brief Container and config for all labels in the sector map.
	 */
	struct Labels {
		/**
		 * @brief Construct the label system object with reference to the parent map.
		 * @param map Reference to the owning SectorMap.
		 */
		explicit Labels(SectorMap &map);

		/// Name of the font (fallback is "orbiteer").
		std::string fontName = "orbiteer";
		/// Font size in pixels.
		int fontSize = 15;
		/// Gap / spacing for label backgrounds or lines.
		float gap = 2.f;
		/// The actual ImGui font pointer for star labels.
		ImFont *starLabelFont = nullptr;
		/// The ImGui font pointer for faction home text.
		ImFont *factionHomeFont = nullptr;
		/// The ImGui font pointer for faction name text.
		ImFont *factionNameFont = nullptr;

		/// The hover area for star labels, updated each frame.
		ImRect starLabelHoverArea;
		/// The hover area for faction home labels.
		ImRect factionHomeHoverArea;
		/// The hover area for faction name labels.
		ImRect factionNameHoverArea;

		/// Color for highlight overlays (using IM_COL32).
		ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
		/// Color for shading or background areas (IM_COL32).
		ImU32 shadeColor = IM_COL32(25, 51, 82, 200);

		/// Reference to the owning SectorMap.
		SectorMap &map;

		/// The array of label objects, each containing data on positioning, text, etc.
		std::vector<std::unique_ptr<Label>> array;
	};

	Labels m_labels;      ///< Manages all label text, fonts, and rendering.
	bool m_hideLabels;    ///< If true, label rendering is suppressed.
};

#endif /* _SECTORMAP_H */
