#include "BaseSphere.h"
#include "Body.h"
#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "lua/Lua.h"
#include "lua/LuaEvent.h"
#include "ModelCache.h"
#include "NavLights.h"
#include "Pi.h"
#include "Player.h"
#include "ship/PlayerShipController.h"
#include "SDL_hints.h"
#include "Shields.h"
#include "Ship.h"
#include "ShipAICmd.h"
#include "Space.h"
#include "SpaceStationType.h"
#include "TerrainBody.h"
#include "pigui/LuaPiGui.h"

#include "doctest.h"

template <typename T>
struct fmt::formatter<vector3<T>> : formatter<T> {

	std::string_view fwdFormat;

	constexpr auto parse(fmt::format_parse_context& ctx)
	{
		// presumably expanding to enclosing curly braces
		auto end = fmt::formatter<T>::parse(ctx) + 1;
		auto begin = ctx.begin() - 2;
		fwdFormat = std::string_view(begin, end - begin);
		return end - 1;
	}

	format_context::iterator format(const vector3<T> &v, format_context& ctx) const
	{
		fmt::format_to(ctx.out(), fwdFormat, v.x);
		fmt::format_to(ctx.out(), " ");
		fmt::format_to(ctx.out(), fwdFormat, v.y);
		fmt::format_to(ctx.out(), " ");
		return fmt::format_to(ctx.out(), fwdFormat, v.z);
	}
};

template <typename T>
struct fmt::formatter<matrix3x3<T>>: formatter<T> {

	std::string_view fwdFormat;

	constexpr auto parse(fmt::format_parse_context& ctx)
	{
		// presumably expanding to enclosing curly braces
		auto end = fmt::formatter<T>::parse(ctx) + 1;
		auto begin = ctx.begin() - 2;
		fwdFormat = std::string_view(begin, end - begin);
		return end - 1;
	}

	format_context::iterator format(const matrix3x3<T> &m, format_context& ctx) const
	{
		fmt::format_to(ctx.out(), fwdFormat, m.VectorX());
		fmt::format_to(ctx.out(), " | ");
		fmt::format_to(ctx.out(), fwdFormat, m.VectorY());
		fmt::format_to(ctx.out(), " | ");
		return fmt::format_to(ctx.out(), fwdFormat, m.VectorZ());
	}
};

class TestApp {

public:
	static TestApp &instance()
	{
		static TestApp a;
		if (!a.game) {
			throw std::runtime_error("The application has not started or is already disabled");
		}
		return a;
	}

	template <typename... T>
	void log(T&& ...args)
	{
		if (!enableLogging) return;
		Log::Info(std::forward<T>(args)...);
	}

	void logShip(Ship *s)
	{
		auto p = s->GetPropulsion();
		log("vel: {:.2f}  pos: {:.2f}  thr: {:.2f}  ori: {:.2f}", s->GetVelocity(), s->GetPosition(), p->GetLinThrusterState(), s->GetOrient());
	}

	void shutDown()
	{
		if (!game) return;

		delete(game);
		game = nullptr;

		Pi::game = nullptr; // Pi::App::OnShutdown asserts it
		Pi::GetApp()->Shutdown();
	}

private:
	TestApp()
	{
	   	SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
		std::map<std::string, std::string> options;
		Pi::Init(options, true);

		auto enableLogsEnv = std::getenv("PIONEER_ENABLE_SIMULATION_LOGS");
		if (enableLogsEnv && strncmp(enableLogsEnv, "0", 1)) {
			enableLogging = true;
		}

		ShipType::Init();
		Lua::Init(Pi::GetAsyncJobQueue());
		PiGui::Lua::Init();
		Lua::InitModules();
		LuaEvent::Init();
		FileSystem::Init();
		BaseSphere::Init(Pi::renderer);
		Pi::modelCache = new ModelCache(Pi::renderer);
		Shields::Init(Pi::renderer);
		NavLights::Init(Pi::renderer);
		SpaceStationType::Init();

		// Gliese 852, binary system, frame 0 is gravpoint
		SystemPath path{ -2, -4, -1, 0, 1 };
		game = new Game(path, 0);

		LuaEvent::Queue("onGameStart");
		LuaEvent::Emit();
	}

	~TestApp()
	{
		shutDown();
	}

public:
	Game *game;

private:
	bool enableLogging = false;
};

class AIMatchVelCommand : public AICommand {
public:
	AIMatchVelCommand(DynamicBody *db, const vector3d &vel) : AICommand(db, AICommand::CMD_NONE), m_vel(vel) {}

private:
	bool TimeStepUpdate() override {

		auto s = static_cast<Ship*>(m_dBody);

		if (s->GetVelocity() == m_vel) return true;

		s->AIMatchVel(m_vel);
		return false;
	}
	vector3d m_vel;
};

TEST_CASE("simulation_tests")
{
	TestApp &app = TestApp::instance();

	auto eps = 0.001;
	app.game->SetTimeAccel(Game::TIMEACCEL_1X);

	SUBCASE("aimatchvel")
	{
		Ship *s = new Ship("sinonatrix");
		REQUIRE(s);

		app.game->GetSpace()->AddBody(s);

		s->SetFrame(0);
		auto fb = Frame::GetFrame(s->GetFrame());
		// don't want to position the test ship inside something
		REQUIRE(fb->GetSystemBody()->GetType() == SystemBody::TYPE_GRAVPOINT);

		s->SetPosition({ 0, 0, 0 });
		s->SetVelocity({ 0, 0, 0 });

		// lower the bow of the ship by 45 degrees, so that it will have to gain
		// speed  simultaneously with a powerful rear thruster and the weakest upper
		s->SetOrient(matrix3x3d::RotateX(DEG2RAD(45.0)));

		// update ship stats
		app.game->TimeStep(app.game->GetTimeStep());

		s->SetAICommand(new AIMatchVelCommand(s, { 0, -100, 0 }));

		SUBCASE("aimatchvel_stable_direction")
		{
			app.game->SetTimeAccel(Game::TIMEACCEL_10X);

			auto p = s->GetPropulsion();

			for (int i = 0; i < 80; ++i) {

				app.logShip(s);

				app.game->TimeStep(app.game->GetTimeStep());

				// check weak thrusters somewhere in the process
				// if they are not fully loaded, AIMatchVel is not fully effective
				if (i == 10) {
					CHECK(abs(p->GetLinThrusterState().y + 1.0) < eps);
				}
			}

			// we want to accelerate strictly in the specified direction
			CHECK(s->GetVelocity().xz().Length() < eps);
			CHECK(s->GetPosition().xz().Length() < eps);
			CHECK(abs(s->GetVelocity().y + 100) < eps);
		}

		SUBCASE("aimatchvel_gain_speed_in_one_frame")
		{
			app.game->SetTimeAccel(Game::TIMEACCEL_10000X);

			app.logShip(s);
			app.game->TimeStep(app.game->GetTimeStep());
			app.logShip(s);

			CHECK(s->GetVelocity().xz().Length() < eps);
			CHECK(s->GetPosition().xz().Length() < eps);
			CHECK(abs(s->GetVelocity().y + 100) < eps);
		}

		app.game->GetSpace()->KillBody(s);
		app.game->TimeStep(app.game->GetTimeStep());
	}

	SUBCASE("flight_assist_in_gravity")
	{
		auto p = Pi::game->GetPlayer();

		FrameId starFrameId = 1;
		Frame *starFrame = Frame::GetFrame(starFrameId);
		REQUIRE(starFrame);

		Body *astro = starFrame->GetBody();
		CHECK(astro->GetType() == ObjectType::STAR);

		TerrainBody *t = static_cast<TerrainBody*>(astro);
		double radius = t->GetSystemBody()->GetRadius();
		CHECK(radius > 100000000); // probably star

		// place the player in the star frame
		// above the pole, in a gravitational field that the ship can handle
		vector3d niceGravityPos{ 0.0, radius * 8.0, 0.0 };
		p->SetFrame(starFrameId);
		p->SetPosition(niceGravityPos);
		p->SetVelocity({});
		p->SetOrient(matrix3x3d::Identity);
		app.game->TimeStep(app.game->GetTimeStep());

		// check the hovering

		p->GetPlayerController()->ToggleCruise();

		auto maintainHeight = p->GetPosition().y;

		for (int i = 0; i < 60; ++i) {
			app.logShip(p);
			app.game->TimeStep(app.game->GetTimeStep());
		}

		CHECK(abs(p->GetPosition().y - maintainHeight) < eps);

		std::string thrustLeftAxis{ "BindAxisThrustLeft" };
		REQUIRE(Pi::input->HasAxisBinding(thrustLeftAxis));

		auto thrustLeft = Pi::input->GetAxisBinding(thrustLeftAxis);
		thrustLeft->SetValue(1.0);
		// to apply thrust control to the ship
		Pi::input->NewFrame();
		Pi::input->DispatchEvents();

		for (int i = 0; i < 60; ++i) {
			app.logShip(p);
			app.game->TimeStep(app.game->GetTimeStep());
		}

		// must gain strictly horizontal speed without sagging
		CHECK(abs(p->GetVelocity().x) > 3.0); // does thrust even work
		CHECK(abs(p->GetPosition().y - maintainHeight) < eps);
	}

	SUBCASE("shutdown_simulation")
	{
		app.shutDown();
		StringTable::Get()->Reclaim(true);
	}
}
