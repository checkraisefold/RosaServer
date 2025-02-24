#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "engine.h"
#include "hooks.h"
#include "sol/sol.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../cpp-httplib/httplib.h"

#define LUA_ENTRY_FILE "main/init.lua"
#define LUA_PREFIX "\033[34;1;4m[RosaServer/Lua]\033[0m "
#define RS_PREFIX "\033[35;1;4m[RosaServer]\033[0m "
#define SUBROSA_PREFIX "\033[31;1;4m[Sub Rosa]\033[0m "

#define RESET_REASON_BOOT 0
#define RESET_REASON_ENGINECALL 1
#define RESET_REASON_LUARESET 2
#define RESET_REASON_LUACALL 3

extern bool initialized;
extern bool shouldReset;

extern sol::state* lua;
extern std::string hookMode;

extern sol::table* accountDataTables[maxNumberOfAccounts];
extern sol::table* playerDataTables[maxNumberOfPlayers];
extern sol::table* humanDataTables[maxNumberOfHumans];
extern sol::table* itemDataTables[maxNumberOfItems];
extern sol::table* vehicleDataTables[maxNumberOfVehicles];
extern sol::table* bodyDataTables[maxNumberOfRigidBodies];

enum LuaRequestType { get, post };

struct LuaHTTPRequest {
	LuaRequestType type;
	std::string scheme;
	std::string path;
	std::shared_ptr<sol::protected_function> callback;
	std::string contentType;
	std::string body;
	httplib::Headers headers;
};

struct LuaHTTPResponse {
	std::shared_ptr<sol::protected_function> callback;
	bool responded;
	int status;
	std::string body;
	httplib::Headers headers;
};

extern std::mutex stateResetMutex;

void printLuaError(sol::error* err);
bool noLuaCallError(sol::protected_function_result* res);
bool noLuaCallError(sol::load_result* res);
void hookAndReset(int reason);

void defineThreadSafeAPIs(sol::state* state);
void luaInit(bool redo = false);

namespace Lua {
void print(sol::variadic_args va, sol::this_state s);
void flagStateForReset(const char* mode);

Vector Vector_();
Vector Vector_3f(float x, float y, float z);
RotMatrix RotMatrix_(float x1, float y1, float z1, float x2, float y2, float z2,
                     float x3, float y3, float z3);

namespace http {
sol::object getSync(const char* scheme, const char* path, sol::table headers,
                    sol::this_state s);
sol::object postSync(const char* scheme, const char* path, sol::table headers,
                     std::string body, const char* contentType,
                     sol::this_state s);
};  // namespace http

namespace hook {
bool enable(std::string name);
bool disable(std::string name);
void clear();
};  // namespace hook

namespace physics {
sol::table lineIntersectLevel(Vector* posA, Vector* posB, bool onlyCity);
sol::table lineIntersectHuman(Human* man, Vector* posA, Vector* posB,
                              float padding);
sol::table lineIntersectVehicle(Vehicle* vcl, Vector* posA, Vector* posB,
                                bool includeWheels);
sol::object lineIntersectLevelQuick(Vector* posA, Vector* posB, bool onlyCity,
                                    sol::this_state s);
sol::object lineIntersectHumanQuick(Human* man, Vector* posA, Vector* posB,
                                    float padding, sol::this_state s);
sol::object lineIntersectVehicleQuick(Vehicle* vcl, Vector* posA, Vector* posB,
                                      bool includeWheels, sol::this_state s);
std::tuple<sol::object, sol::object> lineIntersectAnyQuick(
    Vector* posA, Vector* posB, Human* ignoreHuman, float humanPadding,
    bool includeWheels, sol::this_state s);
sol::object lineIntersectTriangle(Vector* outPos, Vector* normal, Vector* posA,
                                  Vector* posB, Vector* triA, Vector* triB,
                                  Vector* triC, sol::this_state s);
void garbageCollectBullets();
void createBlock(int blockX, int blockY, int blockZ, unsigned int flags);
unsigned int getBlock(int blockX, int blockY, int blockZ);
void deleteBlock(int blockX, int blockY, int blockZ);
};  // namespace physics

namespace itemTypes {
int getCount();
sol::table getAll();
ItemType* getByIndex(sol::table self, unsigned int idx);
ItemType* getByName(const char* name);
};  // namespace itemTypes

namespace items {
int getCount();
sol::table getAll();
Item* getByIndex(sol::table self, unsigned int idx);
Item* create(ItemType* type, Vector* pos, RotMatrix* rot);
Item* createVel(ItemType* typee, Vector* pos, Vector* vel, RotMatrix* rot);
Item* createRope(Vector* pos, RotMatrix* rot);
};  // namespace items

namespace vehicleTypes {
int getCount();
sol::table getAll();
VehicleType* getByIndex(sol::table self, unsigned int idx);
VehicleType* getByName(const char* name);
};  // namespace vehicleTypes

namespace vehicles {
int getCount();
sol::table getAll();
sol::table getNonTrafficCars();
sol::table getTrafficCars();
Vehicle* getByIndex(sol::table self, unsigned int idx);
Vehicle* create(VehicleType* type, Vector* pos, RotMatrix* rot, int color);
Vehicle* createVel(VehicleType* type, Vector* pos, Vector* vel, RotMatrix* rot,
                   int color);
};  // namespace vehicles

namespace chat {
Event* announce(const char* message);
Event* tellAdmins(const char* message);
};  // namespace chat

namespace accounts {
void save();
int getCount();
sol::table getAll();
Account* getByPhone(int phone);
Account* getByIndex(sol::table self, unsigned int idx);
};  // namespace accounts

namespace players {
int getCount();
sol::table getAll();
Player* getByPhone(int phone);
sol::table getNonBots();
sol::table getBots();
Player* getByIndex(sol::table self, unsigned int idx);
Player* createBot();
};  // namespace players

namespace humans {
int getCount();
sol::table getAll();
Human* getByIndex(sol::table self, unsigned int idx);
Human* create(Vector* pos, RotMatrix* rot, Player* ply);
};  // namespace humans

namespace bullets {
unsigned int getCount();
sol::table getAll();
Bullet* create(int type, Vector* pos, Vector* vel, Player* ply);
};  // namespace bullets

namespace rigidBodies {
int getCount();
sol::table getAll();
RigidBody* getByIndex(sol::table self, unsigned int idx);
};  // namespace rigidBodies

namespace bonds {
int getCount();
sol::table getAll();
Bond* getByIndex(sol::table self, unsigned int idx);
};  // namespace bonds

namespace streets {
int getCount();
sol::table getAll();
Street* getByIndex(sol::table self, unsigned int idx);
};  // namespace streets

namespace intersections {
int getCount();
sol::table getAll();
StreetIntersection* getByIndex(sol::table self, unsigned int idx);
};  // namespace intersections

namespace trafficCars {
int getCount();
sol::table getAll();
TrafficCar* getByIndex(sol::table self, unsigned int idx);
void createMany(int amount);
};  // namespace trafficCars

namespace buildings {
int getCount();
sol::table getAll();
Building* getByIndex(sol::table self, unsigned int idx);
};  // namespace buildings

namespace events {
int getCount();
sol::table getAll();
Event* getByIndex(sol::table self, unsigned int idx);
Event* createBullet(int bulletType, Vector* pos, Vector* vel, Item* item);
Event* createBulletHit(int hitType, Vector* pos, Vector* normal);
Event* createMessage(int messageType, const char* message, int speakerID,
                     int volumeLevel);
Event* createSound(int soundType, Vector* pos, float volume, float pitch);
Event* createSoundSimple(int soundType, Vector* pos);
Event* createSoundItem(int soundType, Item* item, float volume, float pitch);
Event* createSoundItemSimple(int soundType, Item* item);
Event* createExplosion(Vector* pos);
};  // namespace events

namespace os {
sol::table listDirectory(std::string_view path, sol::this_state s);
bool createDirectory(std::string_view path);
double getLastWriteTime(std::string_view path);
double realClock();
void exit();
void exitCode(int code);
};  // namespace os

namespace memory {
extern uintptr_t baseAddress;
uintptr_t getBaseAddress();
uintptr_t getAddressOfConnection(Connection* address);
uintptr_t getAddressOfAccount(Account* address);
uintptr_t getAddressOfPlayer(Player* address);
uintptr_t getAddressOfHuman(Human* address);
uintptr_t getAddressOfItemType(ItemType* address);
uintptr_t getAddressOfItem(Item* address);
uintptr_t getAddressOfVehicleType(VehicleType* address);
uintptr_t getAddressOfVehicle(Vehicle* address);
uintptr_t getAddressOfBullet(Bullet* address);
uintptr_t getAddressOfBone(Bone* address);
uintptr_t getAddressOfRigidBody(RigidBody* address);
uintptr_t getAddressOfBond(Bond* address);
uintptr_t getAddressOfAction(Action* address);
uintptr_t getAddressOfMenuButton(MenuButton* address);
uintptr_t getAddressOfStreetLane(StreetLane* address);
uintptr_t getAddressOfStreet(Street* address);
uintptr_t getAddressOfStreetIntersection(StreetIntersection* address);
uintptr_t getAddressOfWheel(Wheel* address);
uintptr_t getAddressOfInventorySlot(InventorySlot* address);
std::string toHexByte(int8_t value);
std::string toHexShort(int16_t value);
std::string toHexInt(int32_t value);
std::string toHexLong(int64_t value);
std::string toHexFloat(float value);
std::string toHexDouble(double value);
std::string toHexString(std::string value);
int8_t readByte(uintptr_t address);
uint8_t readUByte(uintptr_t address);
int16_t readShort(uintptr_t address);
uint16_t readUShort(uintptr_t address);
int32_t readInt(uintptr_t address);
uint32_t readUInt(uintptr_t address);
int64_t readLong(uintptr_t address);
uint64_t readULong(uintptr_t address);
float readFloat(uintptr_t address);
double readDouble(uintptr_t address);
std::string readBytes(uintptr_t address, size_t count);
void writeByte(uintptr_t address, int8_t data);
void writeUByte(uintptr_t address, uint8_t data);
void writeShort(uintptr_t address, int16_t data);
void writeUShort(uintptr_t address, uint16_t data);
void writeInt(uintptr_t address, int32_t data);
void writeUInt(uintptr_t address, uint32_t data);
void writeLong(uintptr_t address, int64_t data);
void writeULong(uintptr_t address, uint64_t data);
void writeFloat(uintptr_t address, float data);
void writeDouble(uintptr_t address, double data);
void writeBytes(uintptr_t address, std::string_view bytes);
}  // namespace memory
};  // namespace Lua

std::string addressFromInteger(unsigned int address);
