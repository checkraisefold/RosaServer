#include "hooks.h"
#include "api.h"
#include "console.h"

static constexpr int httpThreadCount = 2;

int h_subrosa_puts(const char* str) {
	std::ostringstream stream;

	stream << SUBROSA_PREFIX;
	stream << str;
	stream << "\n";

	Console::log(stream.str());

	return 1;
}

int h_subrosa___printf_chk(int flag, const char* format, ...) {
	va_list arguments;
	va_start(arguments, format);

	char buffer[256];
	vsnprintf(buffer, 256, format, arguments);

	std::ostringstream stream;

	stream << SUBROSA_PREFIX;
	stream << buffer;

	Console::log(stream.str());

	va_end(arguments);
	return 0;
}

void h_resetgame() {
	if (!initialized) {
		initialized = true;

		Console::log(RS_PREFIX "Engine ready...\n");

		luaInit();

		Console::log(RS_PREFIX "Initializing input...\n");
		Console::init();

		Console::log(RS_PREFIX "Starting HTTP threads...\n");
		for (int i = 0; i < httpThreadCount; i++) {
			std::thread thread(HTTPThread);
			thread.detach();
		}

		Console::log(RS_PREFIX "Ready!\n");
		hookAndReset(RESET_REASON_BOOT);
	} else {
		hookAndReset(RESET_REASON_ENGINECALL);
	}
}

void h_logicsimulation() {
	if (shouldReset) {
		shouldReset = false;
		luaInit(true);

		hookAndReset(RESET_REASON_LUARESET);
	}

	bool noParent = false;
	sol::protected_function hookFunc = (*lua)["hook"]["run"];

	if (Console::shouldExit) {
		if (hookFunc != sol::nil) {
			auto res = hookFunc("InterruptSignal");
			noLuaCallError(&res);
		}
		exit(EXIT_SUCCESS);
		return;
	}

	if (hookFunc != sol::nil) {
		auto res = hookFunc("Logic");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationHook);
			Engine::logicSimulation();
		}
		if (hookFunc != sol::nil) {
			auto res = hookFunc("PostLogic");
			noLuaCallError(&res);
		}
	}

	{
		std::lock_guard<std::mutex> guard(Console::commandQueueMutex);
		while (!Console::commandQueue.empty()) {
			if (hookFunc != sol::nil) {
				auto res = hookFunc("ConsoleInput", Console::commandQueue.front());
				noLuaCallError(&res);
			}
			Console::commandQueue.pop();
		}
	}

	while (true) {
		responseQueueMutex.lock();
		if (responseQueue.empty()) {
			responseQueueMutex.unlock();
			break;
		}
		auto res = responseQueue.front();
		responseQueue.pop();
		responseQueueMutex.unlock();

		if (hookFunc != sol::nil) {
			if (res.responded) {
				sol::table table = lua->create_table();
				table["status"] = res.status;
				table["body"] = res.body;

				sol::table headers = lua->create_table();
				for (const auto& h : res.headers) headers[h.first] = h.second;
				table["headers"] = headers;

				auto resf = res.callback->call(table);
				noLuaCallError(&resf);
			} else {
				auto resf = res.callback->call();
				noLuaCallError(&resf);
			}
		}
	}

	if (Console::isAwaitingAutoComplete()) {
		if (hookFunc != sol::nil) {
			auto data = lua->create_table();
			data["response"] = Console::getAutoCompleteInput();

			auto res = hookFunc("ConsoleAutoComplete", data);
			noLuaCallError(&res);

			std::string response = data["response"];
			Console::respondToAutoComplete(response);
		} else {
			Console::respondToAutoComplete(Console::getAutoCompleteInput());
		}
	}
}

void h_logicsimulation_race() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicRace");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationRaceHook);
			Engine::logicSimulationRace();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicRace");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_round() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicRound");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationRoundHook);
			Engine::logicSimulationRound();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicRound");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_world() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicWorld");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationWorldHook);
			Engine::logicSimulationWorld();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicWorld");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_terminator() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicTerminator");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationTerminatorHook);
			Engine::logicSimulationTerminator();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicTerminator");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_coop() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicCoop");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationCoopHook);
			Engine::logicSimulationCoop();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicCoop");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_versus() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicVersus");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicSimulationVersusHook);
			Engine::logicSimulationVersus();
		}
		if (func != sol::nil) {
			auto res = func("PostLogicVersus");
			noLuaCallError(&res);
		}
	}
}

void h_logic_playeractions(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerActions", &Engine::players[playerID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&logicPlayerActionsHook);
			Engine::logicPlayerActions(playerID);
		}
		if (func != sol::nil) {
			auto res = func("PostPlayerActions", &Engine::players[playerID]);
			noLuaCallError(&res);
		}
	}
}

void h_physicssimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("Physics");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&physicsSimulationHook);
			Engine::physicsSimulation();
		}
		if (func != sol::nil) {
			auto res = func("PostPhysics");
			noLuaCallError(&res);
		}
	}
}

int h_serverrecv() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("InPacket");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int ret;
		{
			subhook::ScopedHookRemove remove(&serverReceiveHook);
			ret = Engine::serverReceive();
		}
		if (func != sol::nil) {
			auto res = func("PostInPacket");
			noLuaCallError(&res);
		}
		return ret;
	}
	return -1;
}

void h_serversend() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("SendPacket");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&serverSendHook);
			Engine::serverSend();
		}
		if (func != sol::nil) {
			auto res = func("PostSendPacket");
			noLuaCallError(&res);
		}
	}
}

void h_bulletsimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsBullets");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&bulletSimulationHook);
			Engine::bulletSimulation();
		}
		if (func != sol::nil) {
			auto res = func("PostPhysicsBullets");
			noLuaCallError(&res);
		}
	}
}

void h_saveaccountsserver() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("AccountsSave");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&saveAccountsServerHook);
			Engine::saveAccountsServer();
		}
		if (func != sol::nil) {
			auto res = func("PostAccountsSave");
			noLuaCallError(&res);
		}
	}
}

int h_createaccount_jointicket(int identifier, unsigned int ticket) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("AccountTicketBegin", identifier, ticket);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int id;
		{
			subhook::ScopedHookRemove remove(&createAccountByJoinTicketHook);
			id = Engine::createAccountByJoinTicket(identifier, ticket);
		}
		if (func != sol::nil) {
			auto res = func("AccountTicketFound",
			                id == -1 ? nullptr : &Engine::accounts[id]);
			noParent = false;
			if (noLuaCallError(&res)) noParent = (bool)res;

			if (!noParent) {
				auto res = func("PostAccountTicket",
				                id == -1 ? nullptr : &Engine::accounts[id]);
				noLuaCallError(&res);
				return id;
			}
			return -1;
		}
		return id;
	}
	return -1;
}

void h_server_sendconnectreponse(unsigned int address, unsigned int port,
                                 const char* message) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];

	auto addressString = addressFromInteger(address);

	auto data = lua->create_table();
	data["message"] = message;
	std::string newMessage;

	if (func != sol::nil) {
		auto res = func("SendConnectResponse", addressString, port, data);
		if (noLuaCallError(&res)) {
			noParent = (bool)res;
			newMessage = data["message"];
			message = newMessage.c_str();
		}
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&serverSendConnectResponseHook);
			Engine::serverSendConnectResponse(address, port, message);
		}
		if (func != sol::nil) {
			auto res = func("PostSendConnectResponse", addressString, port, data);
			noLuaCallError(&res);
		}
	}
}

int h_createplayer() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerCreate");
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int id;
		{
			subhook::ScopedHookRemove remove(&createPlayerHook);
			id = Engine::createPlayer();

			if (id != -1 && playerDataTables[id]) {
				delete playerDataTables[id];
				playerDataTables[id] = nullptr;
			}
		}
		if (func != sol::nil && id != -1) {
			auto res = func("PostPlayerCreate", &Engine::players[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteplayer(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerDelete", &Engine::players[playerID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&deletePlayerHook);
			Engine::deletePlayer(playerID);

			if (playerDataTables[playerID]) {
				delete playerDataTables[playerID];
				playerDataTables[playerID] = nullptr;
			}
		}
		if (func != sol::nil) {
			auto res = func("PostPlayerDelete", &Engine::players[playerID]);
			noLuaCallError(&res);
		}
	}
}

int h_createhuman(Vector* pos, RotMatrix* rot, int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanCreate", pos, rot, &Engine::players[playerID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int id;
		{
			subhook::ScopedHookRemove remove(&createHumanHook);
			id = Engine::createHuman(pos, rot, playerID);

			if (id != -1 && humanDataTables[id]) {
				delete humanDataTables[id];
				humanDataTables[id] = nullptr;
			}
		}
		if (func != sol::nil && id != -1) {
			auto res = func("PostHumanCreate", &Engine::humans[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deletehuman(int humanID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanDelete", &Engine::humans[humanID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&deleteHumanHook);
			Engine::deleteHuman(humanID);

			if (humanDataTables[humanID]) {
				delete humanDataTables[humanID];
				humanDataTables[humanID] = nullptr;
			}
		}
		if (func != sol::nil) {
			auto res = func("PostHumanDelete", &Engine::humans[humanID]);
			noLuaCallError(&res);
		}
	}
}

int h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ItemCreate", type, pos, rot);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int id;
		{
			subhook::ScopedHookRemove remove(&createItemHook);
			id = Engine::createItem(type, pos, vel, rot);

			if (id != -1 && itemDataTables[id]) {
				delete itemDataTables[id];
				itemDataTables[id] = nullptr;
			}
		}
		if (id != -1 && func != sol::nil) {
			auto res = func("PostItemCreate", &Engine::items[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteitem(int itemID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ItemDelete", &Engine::items[itemID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&deleteItemHook);
			Engine::deleteItem(itemID);

			if (itemDataTables[itemID]) {
				delete itemDataTables[itemID];
				itemDataTables[itemID] = nullptr;
			}
		}
		if (func != sol::nil) {
			auto res = func("PostItemDelete", &Engine::items[itemID]);
			noLuaCallError(&res);
		}
	}
}

int h_createobject(int type, Vector* pos, Vector* vel, RotMatrix* rot,
                   int color) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("VehicleCreate", type, pos, rot, color);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int id;
		{
			subhook::ScopedHookRemove remove(&createVehicleHook);
			id = Engine::createVehicle(type, pos, vel, rot, color);

			if (id != -1 && vehicleDataTables[id]) {
				delete vehicleDataTables[id];
				vehicleDataTables[id] = nullptr;
			}
		}
		if (id != -1 && func != sol::nil) {
			auto res = func("PostVehicleCreate", &Engine::vehicles[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteobject(int vehicleID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("VehicleDelete", &Engine::vehicles[vehicleID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&deleteVehicleHook);
			Engine::deleteVehicle(vehicleID);

			if (vehicleDataTables[vehicleID]) {
				delete vehicleDataTables[vehicleID];
				vehicleDataTables[vehicleID] = nullptr;
			}
		}
		if (func != sol::nil) {
			auto res = func("PostVehicleDelete", &Engine::vehicles[vehicleID]);
			noLuaCallError(&res);
		}
	}
}

int h_createrigidbody(int type, Vector* pos, RotMatrix* rot, Vector* vel,
                      Vector* scale, float mass) {
	int id;
	{
		subhook::ScopedHookRemove remove(&createRigidBodyHook);
		id = Engine::createRigidBody(type, pos, rot, vel, scale, mass);
	}
	if (id != -1 && bodyDataTables[id]) {
		delete bodyDataTables[id];
		bodyDataTables[id] = nullptr;
	}
	return id;
}

int h_linkitem(int itemID, int childItemID, int parentHumanID, int slot) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func(
		    "ItemLink", &Engine::items[itemID],
		    childItemID == -1 ? nullptr : &Engine::items[childItemID],
		    parentHumanID == -1 ? nullptr : &Engine::humans[parentHumanID], slot);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		int worked;
		{
			subhook::ScopedHookRemove remove(&linkItemHook);
			worked = Engine::linkItem(itemID, childItemID, parentHumanID, slot);
		}
		if (func != sol::nil) {
			auto res =
			    func("PostItemLink", &Engine::items[itemID],
			         childItemID == -1 ? nullptr : &Engine::items[childItemID],
			         parentHumanID == -1 ? nullptr : &Engine::humans[parentHumanID],
			         slot, (bool)worked);
			noLuaCallError(&res);
		}
		return worked;
	}
	return 0;
}

void h_item_computerinput(int itemID, unsigned int character) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ItemComputerInput", &Engine::items[itemID], character);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&itemComputerInputHook);
			Engine::itemComputerInput(itemID, character);
		}
		if (func != sol::nil) {
			auto res =
			    func("PostItemComputerInput", &Engine::items[itemID], character);
			noLuaCallError(&res);
		}
	}
}

void h_human_applydamage(int humanID, int bone, int unk, int damage) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanDamage", &Engine::humans[humanID], bone, damage);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&humanApplyDamageHook);
			Engine::humanApplyDamage(humanID, bone, unk, damage);
		}
		if (func != sol::nil) {
			auto res =
			    func("PostHumanDamage", &Engine::humans[humanID], bone, damage);
			noLuaCallError(&res);
		}
	}
}

void h_human_collisionvehicle(int humanID, int vehicleID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanCollisionVehicle", &Engine::humans[humanID],
		                &Engine::vehicles[vehicleID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&humanCollisionVehicleHook);
			Engine::humanCollisionVehicle(humanID, vehicleID);
		}
		if (func != sol::nil) {
			auto res = func("PostHumanCollisionVehicle", &Engine::humans[humanID],
			                &Engine::vehicles[vehicleID]);
			noLuaCallError(&res);
		}
	}
}

void h_human_grabbing(int humanID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanGrabbing", &Engine::humans[humanID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&humanGrabbingHook);
			Engine::humanGrabbing(humanID);
		}
		if (func != sol::nil) {
			auto res = func("PostHumanGrabbing", &Engine::humans[humanID]);
			noLuaCallError(&res);
		}
	}
}

void h_grenadeexplosion(int itemID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("GrenadeExplode", &Engine::items[itemID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&grenadeExplosionHook);
			Engine::grenadeExplosion(itemID);
		}
		if (func != sol::nil) {
			auto res = func("PostGrenadeExplode", &Engine::items[itemID]);
			noLuaCallError(&res);
		}
	}
}

int h_server_playermessage(int playerID, char* message) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerChat", &Engine::players[playerID], message);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		subhook::ScopedHookRemove remove(&serverPlayerMessageHook);
		return Engine::serverPlayerMessage(playerID, message);
	}
	return 1;
}

void h_playerai(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerAI", &Engine::players[playerID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&playerAIHook);
			Engine::playerAI(playerID);
		}
		if (func != sol::nil) {
			auto res = func("PostPlayerAI", &Engine::players[playerID]);
			noLuaCallError(&res);
		}
	}
}

void h_playerdeathtax(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerDeathTax", &Engine::players[playerID]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&playerDeathTaxHook);
			Engine::playerDeathTax(playerID);
		}
		if (func != sol::nil) {
			auto res = func("PostPlayerDeathTax", &Engine::players[playerID]);
			noLuaCallError(&res);
		}
	}
}

void h_addcollision_rigidbody_rigidbody(int aBodyID, int bBodyID,
                                        Vector* aLocalPos, Vector* bLocalPos,
                                        Vector* normal, float a, float b,
                                        float c, float d) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("CollideBodies", &Engine::bodies[aBodyID],
		                &Engine::bodies[bBodyID], aLocalPos, bLocalPos, normal, a,
		                b, c, d);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		subhook::ScopedHookRemove remove(&addCollisionRigidBodyOnRigidBodyHook);
		Engine::addCollisionRigidBodyOnRigidBody(aBodyID, bBodyID, aLocalPos,
		                                         bLocalPos, normal, a, b, c, d);
	}
}

/*
Type:
0 = Chat
1 = Speaking
2 = Item (Phone)
3 = MOTD
4 = To Admins
5 = Billboard
6 = To Player (Crim)
*/
void h_createevent_message(int speakerType, char* message, int speakerID,
                           int distance) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventMessage", speakerType, message, speakerID, distance);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createEventMessageHook);
			Engine::createEventMessage(speakerType, message, speakerID, distance);
		}
		if (func != sol::nil) {
			auto res =
			    func("PostEventMessage", speakerType, message, speakerID, distance);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_updateplayer(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdatePlayer", &Engine::players[id]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createEventUpdatePlayerHook);
			Engine::createEventUpdatePlayer(id);
		}
		if (func != sol::nil) {
			auto res = func("PostEventUpdatePlayer", &Engine::players[id]);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_updateplayer_finance(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdatePlayerFinance", &Engine::players[id]);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createEventUpdatePlayerFinanceHook);
			Engine::createEventUpdatePlayerFinance(id);
		}
		if (func != sol::nil) {
			auto res = func("PostEventUpdatePlayerFinance", &Engine::players[id]);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_updateobject(int vehicleID, int updateType, int partID,
                                Vector* pos, Vector* normal) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdateVehicle", &Engine::vehicles[vehicleID],
		                updateType, partID, pos, normal);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createEventUpdateVehicleHook);
			Engine::createEventUpdateVehicle(vehicleID, updateType, partID, pos,
			                                 normal);
		}
		if (func != sol::nil) {
			auto res = func("PostEventUpdateVehicle", &Engine::vehicles[vehicleID],
			                updateType, partID, pos, normal);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_bullethit(int unk, int hitType, Vector* pos,
                             Vector* normal) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventBulletHit", hitType, pos, normal);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createEventBulletHitHook);
			Engine::createEventBulletHit(unk, hitType, pos, normal);
		}
		if (func != sol::nil) {
			auto res = func("PostEventBulletHit", hitType, pos, normal);
			noLuaCallError(&res);
		}
	}
}

int h_lineintersecthuman(int humanID, Vector* posA, Vector* posB) {
	int didHit;
	{
		subhook::ScopedHookRemove remove(&lineIntersectHumanHook);
		didHit = Engine::lineIntersectHuman(humanID, posA, posB);
	}

	if (!didHit) {
		return didHit;
	}

	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LineIntersectHuman", &Engine::humans[humanID], posA, posB);
		if (noLuaCallError(&res)) noParent = (bool)res;
	}

	return !noParent;
}