#include "Actor.h"

#include "../../../Manager/Manager.h"
#include "../../../Category/Category.h"
#include "../../../Module/Module.h"

Manager* actorManager = nullptr;
auto entityList = std::vector<Actor*>();

Hook_Actor::Hook_Actor(Manager* manager) {
    this->init(manager);
};

typedef void (__thiscall* ActorTick)(void*, Actor*);
ActorTick _ActorTick;

auto actorTick(void* a1, Actor* entity) -> void {
    if(actorManager != nullptr){
        for(auto category : actorManager->getCategories()){
            for(auto mod : category->getModules()){
                if(mod->isEnabled)
                    mod->onActorTick(entity);
            };
        };

        auto instance = Minecraft::getClientInstance();

        if(instance == nullptr)
            entityList.clear();
        
        if(instance->getLocalPlayer() == nullptr)
            entityList.clear();

        if(std::find(entityList.begin(), entityList.end(), entity) != entityList.end())
            entityList.clear();
        
        entityList.push_back(entity);
        actorManager->setEntityList(entityList);
    };

    _ActorTick(a1, entity);
};

auto Hook_Actor::init(Manager* manager) -> void {
    actorManager = manager;

    auto sig = (uintptr_t)NULL;

    switch(Minecraft::sdkVer){
        case MC_VER::v1_17_40_6:
            sig = Mem::findSig("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 60 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B 01");
        break;
    };

    if(sig == NULL)
        return Utils::debugLogF("Unable to create hook for Actor::tick\n");
    
    if(MH_CreateHook((void*)sig, &actorTick, reinterpret_cast<LPVOID*>(&_ActorTick)) == MH_OK){
        Utils::debugLogF("Successfully created hook for Actor::tick - Enabling now...\n");
        MH_EnableHook((void*)sig);
    }
    else {
        Utils::debugLogF("Failed to create hook for Actor::tick\n");
    };
};