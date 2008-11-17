#ifndef _AISHIP_H
#define _AISHIP_H

#include "Ship.h"
#include "ShipType.h"
#include <list>

class AIShip: public Ship {
public:
	OBJDEF(AIShip, Ship, AISHIP);
	AIShip(ShipType::Type shipType): Ship(shipType) {}
	AIShip(): Ship() {}
	void TimeStepUpdate(const float timeStep);
	
	enum Command { DO_NOTHING, DO_KILL };
	void Instruct(enum Command, void *arg);
	void ClearInstructions() { m_todo.clear(); }
	virtual void PostLoadFixup();
protected:
	virtual void Save();
	virtual void Load();
private:
	bool DoKill(const Ship *);

	class Instruction {
	public:
		Command cmd;
		void *arg;
		Instruction(Command c, void *a): cmd(c), arg(a) {}
	};
	std::list<Instruction> m_todo;
};

#endif /* _AISHIP_H */
