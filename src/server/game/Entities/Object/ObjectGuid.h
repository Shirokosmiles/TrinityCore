/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ObjectGuid_h__
#define ObjectGuid_h__

#include "Define.h"
#include "ByteBuffer.h"
#include <deque>
#include <functional>
#include <list>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <vector>

enum TypeID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7,
    TYPEID_AREATRIGGER   = 8
};

#define NUM_CLIENT_OBJECT_TYPES             9

enum TypeMask
{
    TYPEMASK_OBJECT         = 0x0001,
    TYPEMASK_ITEM           = 0x0002,
    TYPEMASK_CONTAINER      = 0x0006,                       // TYPEMASK_ITEM | 0x0004
    TYPEMASK_UNIT           = 0x0008,                       // creature
    TYPEMASK_PLAYER         = 0x0010,
    TYPEMASK_GAMEOBJECT     = 0x0020,
    TYPEMASK_DYNAMICOBJECT  = 0x0040,
    TYPEMASK_CORPSE         = 0x0080,
    TYPEMASK_AREATRIGGER    = 0x0100,
    TYPEMASK_SEER           = TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT
};

enum class HighGuid
{
    Item           = 0x400,                       // blizz 4000
    Container      = 0x400,                       // blizz 4000
    Player         = 0x000,                       // blizz 0000
    GameObject     = 0xF11,                       // blizz F110
    Transport      = 0xF12,                       // blizz F120 // unused according to sniffs GAMEOBJECT_TYPE_TRANSPORT uses Mo_Transport as well
    Unit           = 0xF13,                       // blizz F130
    Pet            = 0xF14,                       // blizz F140
    Vehicle        = 0xF15,                       // blizz F550
    DynamicObject  = 0xF10,                       // blizz F100
    Corpse         = 0xF101,                      // blizz F100
    AreaTrigger    = 0xF102,
    BattleGround   = 0x1F1,
    Mo_Transport   = 0x1FC,                       // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT and GAMEOBJECT_TYPE_TRANSPORT)
    Instance       = 0x1F4,                       // blizz 1F40
    Group          = 0x1F5,
    Guild          = 0x1FF
};

template<HighGuid guid>
struct ObjectGuidTraits
{
    static bool const Global = false;
    static bool const MapSpecific = false;
};

#define GUID_TRAIT_GLOBAL(highguid) \
    template<> struct ObjectGuidTraits<highguid> \
    { \
        static bool const Global = true; \
        static bool const MapSpecific = false; \
    }

#define GUID_TRAIT_MAP_SPECIFIC(highguid) \
    template<> struct ObjectGuidTraits<highguid> \
    { \
        static bool const Global = false; \
        static bool const MapSpecific = true; \
    }

GUID_TRAIT_GLOBAL(HighGuid::Mo_Transport);
GUID_TRAIT_GLOBAL(HighGuid::Group);
GUID_TRAIT_GLOBAL(HighGuid::Instance);
GUID_TRAIT_GLOBAL(HighGuid::BattleGround);
GUID_TRAIT_GLOBAL(HighGuid::Player);
GUID_TRAIT_GLOBAL(HighGuid::Item);
GUID_TRAIT_GLOBAL(HighGuid::Transport);
GUID_TRAIT_GLOBAL(HighGuid::Guild);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::Unit);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::Vehicle);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::Pet);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::GameObject);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::DynamicObject);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::Corpse);
GUID_TRAIT_MAP_SPECIFIC(HighGuid::AreaTrigger);

#undef GUID_TRAIT_GLOBAL
#undef GUID_TRAIT_REALM_SPECIFIC
#undef GUID_TRAIT_MAP_SPECIFIC

class ObjectGuid;
class PackedGuid;

struct PackedGuidReader
{
    explicit PackedGuidReader(ObjectGuid& guid) : Guid(guid) { }
    ObjectGuid& Guid;
};

class TC_GAME_API ObjectGuid
{
    public:
        static ObjectGuid const Empty;

        typedef uint32 LowType;

        template<HighGuid type>
        static typename std::enable_if<ObjectGuidTraits<type>::Global, ObjectGuid>::type Create(LowType counter) { return Global(type, counter); }

        template<HighGuid type>
        static typename std::enable_if<ObjectGuidTraits<type>::MapSpecific, ObjectGuid>::type Create(uint32 entry, LowType counter) { return MapSpecific(type, entry, counter); }

        ObjectGuid() { _data._guid = UI64LIT(0); }
        explicit ObjectGuid(uint64 guid) { _data._guid = guid; }
        ObjectGuid(HighGuid hi, uint32 entry, uint32 counter) { _data._guid = counter ? uint64(counter) | (uint64(entry) << 32) | (uint64(hi) << ((hi == HighGuid::Corpse || hi == HighGuid::AreaTrigger) ? 48 : 52)) : 0; }
        ObjectGuid(HighGuid hi, uint32 counter) { _data._guid = counter ? uint64(counter) | (uint64(hi) << ((hi == HighGuid::Corpse || hi == HighGuid::AreaTrigger) ? 48 : 52)) : 0; }
        ObjectGuid(ObjectGuid const& r) : _data(r._data) { }
        ObjectGuid(ObjectGuid&& r) noexcept : _data(r._data) { }

        ObjectGuid& operator=(ObjectGuid const& r) { _data = r._data; return *this; }
        ObjectGuid& operator=(ObjectGuid&& r) noexcept  { _data = r._data; return *this; }

        operator uint64() const { return _data._guid; }
        PackedGuidReader ReadAsPacked() { return PackedGuidReader(*this); }

        void Set(uint64 guid) { _data._guid = guid; }
        void Clear() { _data._guid = 0; }

        PackedGuid WriteAsPacked() const;

        uint64   GetRawValue() const { return _data._guid; }
        HighGuid GetHigh() const
        {
            HighGuid temp = static_cast<HighGuid>((uint64(_data._guid) >> 48) & 0x0000FFFF);
            return HighGuid((temp == HighGuid::Corpse || temp == HighGuid::AreaTrigger) ? temp : HighGuid(((uint32)temp >> 4) & 0x00000FFF));
        }
        uint32   GetEntry() const { return HasEntry() ? uint32((_data._guid >> 32) & UI64LIT(0x00000000000FFFFF)) : 0; }
        uint32   GetCounter()  const
        {
            return uint32(_data._guid & UI64LIT(0x00000000FFFFFFFF));
        }

        static LowType GetMaxCounter(HighGuid /*high*/)
        {
            return LowType(0xFFFFFFFF);
        }

        ObjectGuid::LowType GetMaxCounter() const { return GetMaxCounter(GetHigh()); }

        uint8& operator[](uint32 index);
        uint8 const& operator[](uint32 index) const;

        bool IsEmpty()             const { return _data._guid == 0; }
        bool IsCreature()          const { return GetHigh() == HighGuid::Unit; }
        bool IsPet()               const { return GetHigh() == HighGuid::Pet; }
        bool IsVehicle()           const { return GetHigh() == HighGuid::Vehicle; }
        bool IsCreatureOrPet()     const { return IsCreature() || IsPet(); }
        bool IsCreatureOrVehicle() const { return IsCreature() || IsVehicle(); }
        bool IsAnyTypeCreature()   const { return IsCreature() || IsPet() || IsVehicle(); }
        bool IsPlayer()            const { return !IsEmpty() && GetHigh() == HighGuid::Player; }
        bool IsUnit()              const { return IsAnyTypeCreature() || IsPlayer(); }
        bool IsItem()              const { return GetHigh() == HighGuid::Item; }
        bool IsGameObject()        const { return GetHigh() == HighGuid::GameObject; }
        bool IsDynamicObject()     const { return GetHigh() == HighGuid::DynamicObject; }
        bool IsCorpse()            const { return GetHigh() == HighGuid::Corpse; }
        bool IsAreaTrigger()       const { return GetHigh() == HighGuid::AreaTrigger; }
        bool IsBattleground()      const { return GetHigh() == HighGuid::BattleGround; }
        bool IsTransport()         const { return GetHigh() == HighGuid::Transport; }
        bool IsMOTransport()       const { return GetHigh() == HighGuid::Mo_Transport; }
        bool IsAnyTypeGameObject() const { return IsGameObject() || IsTransport() || IsMOTransport(); }
        bool IsInstance()          const { return GetHigh() == HighGuid::Instance; }
        bool IsGroup()             const { return GetHigh() == HighGuid::Group; }
        bool IsGuild()             const { return GetHigh() == HighGuid::Guild; }

        static TypeID GetTypeId(HighGuid high)
        {
            switch (high)
            {
                case HighGuid::Item:          return TYPEID_ITEM;
                //case HighGuid::Container:     return TYPEID_CONTAINER; HighGuid::Container==HighGuid::Item currently
                case HighGuid::Unit:          return TYPEID_UNIT;
                case HighGuid::Pet:           return TYPEID_UNIT;
                case HighGuid::Player:        return TYPEID_PLAYER;
                case HighGuid::GameObject:    return TYPEID_GAMEOBJECT;
                case HighGuid::DynamicObject: return TYPEID_DYNAMICOBJECT;
                case HighGuid::Corpse:        return TYPEID_CORPSE;
                case HighGuid::AreaTrigger:   return TYPEID_AREATRIGGER;
                case HighGuid::Mo_Transport:  return TYPEID_GAMEOBJECT;
                case HighGuid::Vehicle:       return TYPEID_UNIT;
                // unknown
                case HighGuid::Instance:
                case HighGuid::BattleGround:
                case HighGuid::Group:
                case HighGuid::Guild:
                default:                      return TYPEID_OBJECT;
            }
        }

        TypeID GetTypeId() const { return GetTypeId(GetHigh()); }

        bool operator!() const { return IsEmpty(); }
        bool operator==(ObjectGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
        bool operator!=(ObjectGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
        bool operator<(ObjectGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }

        static char const* GetTypeName(HighGuid high);
        char const* GetTypeName() const { return !IsEmpty() ? GetTypeName(GetHigh()) : "None"; }
        std::string ToString() const;

    private:
        static bool HasEntry(HighGuid high)
        {
            switch (high)
            {
                case HighGuid::Item:
                case HighGuid::Player:
                case HighGuid::DynamicObject:
                case HighGuid::Corpse:
                case HighGuid::Mo_Transport:
                case HighGuid::Instance:
                case HighGuid::Group:
                    return false;
                case HighGuid::GameObject:
                case HighGuid::Transport:
                case HighGuid::Unit:
                case HighGuid::Pet:
                case HighGuid::Vehicle:
                default:
                    return true;
            }
        }

        bool HasEntry() const { return HasEntry(GetHigh()); }

        static ObjectGuid Global(HighGuid type, LowType counter);
        static ObjectGuid MapSpecific(HighGuid type, uint32 entry, LowType counter);

        explicit ObjectGuid(uint32 const&) = delete;                 // no implementation, used to catch wrong type assignment
        ObjectGuid(HighGuid, uint32, uint64 counter) = delete;       // no implementation, used to catch wrong type assignment
        ObjectGuid(HighGuid, uint64 counter) = delete;               // no implementation, used to catch wrong type assignment

        union
        {
            uint64 _guid;
            uint8 _bytes[sizeof(uint64)];
        } _data;
};

// Some Shared defines
typedef std::set<ObjectGuid> GuidSet;
typedef std::list<ObjectGuid> GuidList;
typedef std::deque<ObjectGuid> GuidDeque;
typedef std::vector<ObjectGuid> GuidVector;
typedef std::unordered_set<ObjectGuid> GuidUnorderedSet;

// minimum buffer size for packed guid is 9 bytes
#define PACKED_GUID_MIN_BUFFER_SIZE 9

class TC_GAME_API PackedGuid
{
    friend TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, PackedGuid const& guid);

    public:
        explicit PackedGuid() : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(0); }
        explicit PackedGuid(uint64 guid) : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(guid); }
        explicit PackedGuid(ObjectGuid guid) : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(guid.GetRawValue()); }

        void Set(uint64 guid) { _packedGuid.wpos(0); _packedGuid.appendPackGUID(guid); }
        void Set(ObjectGuid guid) { _packedGuid.wpos(0); _packedGuid.appendPackGUID(guid.GetRawValue()); }

        std::size_t size() const { return _packedGuid.size(); }

    private:
        ByteBuffer _packedGuid;
};

class TC_GAME_API ObjectGuidGeneratorBase
{
    public:
        ObjectGuidGeneratorBase(ObjectGuid::LowType start = 1) : _nextGuid(start) { }

        virtual void Set(ObjectGuid::LowType val) { _nextGuid = val; }
        virtual ObjectGuid::LowType Generate() = 0;
        ObjectGuid::LowType GetNextAfterMaxUsed() const { return _nextGuid; }
        virtual ~ObjectGuidGeneratorBase() { }

    protected:
        static void HandleCounterOverflow(HighGuid high);
        static void CheckGuidTrigger(ObjectGuid::LowType guid);
        ObjectGuid::LowType _nextGuid;
};

template<HighGuid high>
class ObjectGuidGenerator : public ObjectGuidGeneratorBase
{
    public:
        explicit ObjectGuidGenerator(ObjectGuid::LowType start = 1) : ObjectGuidGeneratorBase(start) { }

        ObjectGuid::LowType Generate() override
        {
            if (_nextGuid >= ObjectGuid::GetMaxCounter(high) - 1)
                HandleCounterOverflow(high);

            if (high == HighGuid::Unit || high == HighGuid::GameObject)
                CheckGuidTrigger(_nextGuid);

            return _nextGuid++;
        }
};

TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid);
TC_GAME_API ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid&       guid);

TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, PackedGuid const& guid);
TC_GAME_API ByteBuffer& operator>>(ByteBuffer& buf, PackedGuidReader const& guid);

inline PackedGuid ObjectGuid::WriteAsPacked() const { return PackedGuid(*this); }

namespace std
{
    template<>
    struct hash<ObjectGuid>
    {
        public:
            size_t operator()(ObjectGuid const& key) const
            {
                return std::hash<uint64>()(key.GetRawValue());
            }
    };
}

#endif // ObjectGuid_h__
