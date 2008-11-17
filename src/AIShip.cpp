#include "libs.h"
#include "AIShip.h"
#include "Serializer.h"
#include "Pi.h"

bool AIShip::DoKill(const Ship *enemy)
{
	/* needs to deal with frames, large distances, and success */
	if (GetFrame() == enemy->GetFrame()) {
		vector3d dir = vector3d::Normalize(enemy->GetPosition() - GetPosition());
		AIFaceDirection(dir);
	}
	return false;
}

void AIShip::TimeStepUpdate(const float timeStep)
{
	bool done = false;

	if (m_todo.size() != 0) {
		Instruction &inst = m_todo.front();
		switch (inst.cmd) {
			case DO_KILL:
				done = DoKill(static_cast<const Ship*>(inst.arg));
				break;
			case DO_NOTHING: done = true; break;
		}
	}
	if (done) { 
		printf("AI '%s' successfully executed %d:'%s'\n", GetLabel().c_str(), m_todo.front().cmd,
				static_cast<Ship*>(m_todo.front().arg)->GetLabel().c_str());
		m_todo.pop_front();
	}

	Ship::TimeStepUpdate(timeStep);
}

void AIShip::Save()
{
	using namespace Serializer::Write;
	Ship::Save();
	wr_int(m_todo.size());
	for (std::list<Instruction>::iterator i = m_todo.begin(); i != m_todo.end(); ++i) {
		wr_int((int)(*i).cmd);
		switch ((*i).cmd) {
			case DO_KILL:
				wr_int(Serializer::LookupBody(static_cast<Ship*>((*i).arg)));
				break;
			case DO_NOTHING: wr_int(0); break;
		}
	}
}

void AIShip::Load()
{
	using namespace Serializer::Read;
	Ship::Load();
	int num = rd_int();
	while (num-- > 0) {
		Command c = (Command)rd_int();
		void *arg = (void*)rd_int();
		printf("COMMAND %d:%p\n", c, arg);
		m_todo.push_back(Instruction(c, arg));
	}
}

void AIShip::PostLoadFixup()
{
	Ship::PostLoadFixup();
	for (std::list<Instruction>::iterator i = m_todo.begin(); i != m_todo.end(); ++i) {
		switch ((*i).cmd) {
			case DO_KILL:
				(*i).arg = Serializer::LookupBody((size_t)(*i).arg);
				break;
			case DO_NOTHING: break;
		}
	}
}

void AIShip::Instruct(enum Command cmd, void *arg)
{
	m_todo.push_back(Instruction(cmd, arg));
}
