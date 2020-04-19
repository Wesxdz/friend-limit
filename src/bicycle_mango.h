#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <SFML/System/Clock.hpp>

#include "specificity.h"
#include "Utils/hole_vector.h"
#include "sun_lambda.h"

using byte = uint8_t;
#define SpecificityDepth 4
using ScheduleSpecificity = Specificity<SpecificityDepth>;

/*
 * Bicycle Mango
 * A hopeful gameplay framework
 * by Wesxdz and lapinozz
 * fast and free for (0b11111111 : while(true))
 *
 *
 *      ***               o000P
 *          \               |.
 *           |_____________/_----____/
 *           // \             \\
 *          //   \             \\       ___
 *     00000/     \             000011 /  v \
 *   00\-|--00     \          00\-|--11  v  /
 *  (0---O---0)     ( λ )----(0--xOx--0)   V
 *   00/-|--00        |-------00/-|--00
 *     00000         ^^^        00000
 *
 *
 * The role of a game engine is to empower the negotiation of imagination.
 * Creating a game should be fun.
 *
 */
class BicycleMango
{
public:
    // The time between the beginning and end of the last loop
    static inline sf::Time delta;
    // Should the Bicycle Mango gameplay loop stop?
    static inline bool brake;

    
    struct SunSchedule
    {
        SunLambda::Id id;
        ScheduleSpecificity specificity;
    };
    static inline std::vector<SunSchedule> schedules;

    static void Loop()
    {
        sf::Clock clock;
        auto startTime = clock.getElapsedTime();
        for (const SunSchedule& schedule : schedules)
        {
            //std::cout << SunLambdaRegistry::GetInstance().Get(schedule.id).name << std::endl;
            SunLambdaRegistry::GetInstance().Get(schedule.id)();
        }

#ifdef HOT_RELOAD
        if(ShouldReloadLambdas)
        {
            ReloadLambdas();
            ShouldReloadLambdas = false;
        }
#endif

        delta = clock.getElapsedTime() - startTime;
    }

#ifdef HOT_RELOAD
    static inline bool ShouldReloadLambdas = false;
    static void ReloadLambdas()
    {
        SunLambdaRegistry::GetInstance().Unload();

        system(HOT_RELOAD_CMAKE " --build " HOT_RELOAD_BUILD_PATH " --target " HOT_RELOAD_TARGET);

        SunLambdaRegistry::GetInstance().Reload();
    }
#endif

    using PropTypeId = size_t;
    using PropIdRaw = size_t;
    // Can novel tuples be formed on this stage?
    using CompatibleConstraint = std::function<bool(Stage&)>;
    // Provide a mapping from a prop considered for tuple to non consumed partial static
    using PartialStaticIndicators = std::unordered_map<PropTypeId, std::function<bool(SunLambda::Id, const Stage&)>>;
    
    struct NovelTupleCreator
    {
        CompatibleConstraint compatible;
        PartialStaticIndicators stageConstraints;
    };

    static inline std::set<SunLambda::Id> emerges;
    static void Emerge(SunLambda::Id id, CompatibleConstraint compatible, PartialStaticIndicators stageConstraints)
    {
        emerges.insert(id);
        novelTupleCreators[id] = {compatible, stageConstraints};
    }
    
    // Shared between emerge/plan/breakup: we assume a SunLambda cannot have multiple NovelTupleCreators for now
    static inline std::unordered_map<SunLambda::Id, NovelTupleCreator> novelTupleCreators;
    // This data structure is an augment of potential neighbors when searching for props to form novel tuples
    static inline std::unordered_map<SunLambda::Id, std::vector<std::vector<PropIdRaw>>> partialStatics;
    
    static void Plan(SunLambda::Id id, const ScheduleSpecificity& specificity, CompatibleConstraint compatible = [](Stage& stage){return true;}, PartialStaticIndicators stageConstraints = {})
    {
        SunSchedule schedule{id, specificity};

        auto it = std::upper_bound(schedules.begin(), schedules.end(), schedule,
        [](const SunSchedule& a, const SunSchedule& b) -> bool {
            return a.specificity < b.specificity;
        });

        schedules.insert(it, schedule);
        novelTupleCreators[id] = {compatible, stageConstraints};
    }
    
    static inline std::set<SunLambda::Id> breakups;
    static void Breakup(SunLambda::Id id, CompatibleConstraint compatible, PartialStaticIndicators stageConstraints)
    {
        breakups.insert(id);
        novelTupleCreators[id] = {compatible, stageConstraints};
    }

    template<typename T>
    struct PropId { PropIdRaw id; };

    // I decided to place PropTypeId before Group this data structure because it is more commonly used this way in the code, even though the console works by query by group
    // TODO Will we need this later for faster console querying??
//     using GroupTypeInstance = std::unordered_map<PropTypeId, std::unordered_map<Group, std::set<std::pair<Instance, PropIdRaw>>>>;
    struct InstanceComparator
    {
        bool operator() (const auto& a, const auto& b) const
        {
            return a.group < b.group || a.instance < b.instance;
        }
    };

//     static inline std::unordered_map<PropTypeId, std::unordered_map<PropIdRaw, std::set<Stage, InstanceComparator>>> ptpsq; // Prop type prop stages query
    // SHOULD WE ONLY STORE THIS INFORMATION FOR PARTIAL STATICS OR EVERYTHING???
    static inline std::unordered_map<PropTypeId, std::unordered_map<Group, std::unordered_map<Instance, std::set<PropIdRaw>>>> ptgid;
    
    struct GlobalPropId {PropTypeId typeId;PropIdRaw id;};
    friend inline bool operator< (const GlobalPropId& lhs, const GlobalPropId& rhs){ return lhs.typeId < rhs.typeId && lhs.id < rhs.id; }


    // Rather ironically, Typeset is a vector because the prop types must be delivered in a certain order to the SunLambda functor despite the conceptual set of props being acted upon
    using Typeset = std::vector<PropTypeId>;
    // All SunLambda and Jolt typeset signatures used (we should only keep track of novel tuples of typesets that are used by gameplay programmers by adding considered typesets in the DefineSunLambda constructor)
    
    static inline std::unordered_map<PropTypeId, std::set<Typeset>> mappedPropTupleTypesets;
    static inline std::set<Typeset> globalPropTupleTypesets;

    // Each typeset stores a map of types to props which have been added and have not been formed into novel tuples of this typeset yet
    // Partial static indicators that return true
    static inline std::map<Typeset, std::unordered_map<PropTypeId, std::vector<PropIdRaw>>> stagingPropTuples;
    
    static inline std::map<Typeset, std::vector<std::vector<GlobalPropId>>> novelTuples;
    static inline std::unordered_map<SunLambda::Id, Typeset> sunLambdaTypesets;
    static inline std::map<Typeset, std::vector<SunLambda::Id>> typesetSunLambdas;

    template<typename... PTypes>
    static void ConsiderTypeset(SunLambda::Id id)
    {
        Typeset typeset = {BicycleMango::GetPropTypeId<std::decay_t<PTypes>>()...};
        sunLambdaTypesets[id] = typeset;
        typesetSunLambdas[typeset].push_back(id);
        for (PropTypeId ptid : typeset)
        {
            BicycleMango::globalPropTupleTypesets.insert(typeset);
            BicycleMango::mappedPropTupleTypesets[ptid].insert(typeset);
        }
    }
    
    template <typename... PTypes>
    static void CallJolt(void (*functor)(PTypes...), SunLambda::Id id, std::vector<PropIdRaw>& sunData)
    {
        CallJolt<PTypes...>(functor, id, sunData, std::index_sequence_for<PTypes...> {});
        
    }
    
    template <typename... PTypes, std::size_t ... Is>
    static void CallJolt(void (*functor)(PTypes...), SunLambda::Id id, std::vector<PropIdRaw>& sunData, std::index_sequence<Is...> seq)
    {
//         std::cout << "Calling jolt with " << sizeof...(PTypes) << " props and " << sunData.size() << " sun data!" << std::endl;
        functor(GetProps<std::decay_t<PTypes>>()[sunData[Is]]...);
    }

    // Prop ----------------

    template <typename T>
    static constexpr PropTypeId GetPropTypeId()
    {
        return std::type_index(typeid(T)).hash_code();
    }
    
    static inline std::unordered_map<Group, IdPool<Instance>> instanceBuffer;
    
    static Stage Next(Group group)
    {
        return {group, instanceBuffer[group].next()};
    }

    template<typename PropType>
    static void AddPropStage(PropId<PropType> propId, const Stage& stage)
    {
        instanceBuffer[stage.group].nextId = stage.instance + 1; // This is kind of a hack, and could cause some large sections of unused ids potentially but I think it will work
//         ptpsq[GetPropTypeId<PropType>()][propId.id].insert(stage);
        ptgid[GetPropTypeId<PropType>()][stage.group][stage.instance].insert(propId.id);
    }

    template<typename PropType>
    static void AddPropStages(PropId<PropType> propId, const GroupSet& stages)
    {
        for(const Stage& stage : stages)
        {
            AddPropStage(propId, stage);
        }
    }
    
    // SunLambda::Id id, 
    static inline bool ArePropsCompatible(GlobalPropId a, GlobalPropId b)
    {
        // TODO If SunLambdas are able to specify (and poentially even change at runtime!) a CompatibleConstraint,
        // we need to keep track of which novel tuples of that SunLambda's typeset each SunLambda should act upon
        return true;
    }
    
    template<typename PropType>
    static PropType* AddProp(const GroupSet& stages)
    {
        auto [id, prop] = GetProps<PropType>().next();
        prop.id = id;

        PropId<PropType> propId{id};
        AddPropStages(propId, stages);
        
        auto propTypeId = GetPropTypeId<PropType>();
        auto typesetsWithAddedPropType = BicycleMango::mappedPropTupleTypesets[propTypeId];
                        
        for (auto typeset_it = typesetsWithAddedPropType.begin(); typeset_it != typesetsWithAddedPropType.end(); ++typeset_it)
        {
            // Check if a novel tuple is formed with this prop from the staged prop neighbors of each Typeset that propTypeId is in!
            // Should partial statics be part of potentialNeighbors? NO, because then we'd have to copy rather than using a ref
            std::unordered_map<PropTypeId, std::vector<PropIdRaw>>& potentialNeighbors = BicycleMango::stagingPropTuples[(*typeset_it)];
            std::unordered_map<PropTypeId, std::vector<PropIdRaw>> compatibleNeighbors;
            bool novelTupleRuledOut = false;
            
            // We need to make sure that potentialNeighbors has at least one prop of each type in this typeset other than the type of prop being added, otherwise we can rule out creating a novel tuple
            for (PropTypeId ptid : (*typeset_it))
            {
                if (ptid != propTypeId && potentialNeighbors[ptid].empty())
                {
                    novelTupleRuledOut = true;
                    break;
                }
            }
//             std::cout << "Poential neighbors are " << potentialNeighbors.size() << std::endl;
            if (!novelTupleRuledOut)
            {
                for (auto ptid_it = potentialNeighbors.begin(); ptid_it != potentialNeighbors.end(); ++ptid_it)
                {
                    if ((*ptid_it).first != propTypeId) // is prop of other type
                    {
                        // TODO: Do they share a stage?
                        // TODO: Do they have no differing stages of shared groups?
                        // Store which props are not part of novel tuples yet and only search through those, UNLESS a prop has multiple instances of a group
                        
                        for (auto neighborId : (*ptid_it).second)
                        {
                            //TODO For each SunLambda Id in Typeset, track if it is compatible with this tuple
                            if (ArePropsCompatible({propTypeId, id}, {(*ptid_it).first, neighborId}))
                            {
                                compatibleNeighbors[(*ptid_it).first].push_back(neighborId);
                            }
                        }
                        if (compatibleNeighbors[(*ptid_it).first].empty()) 
                        {
                            novelTupleRuledOut = true;
                            break; // No compatible neighbors of ptid to pair with
                        }
                    }
                }
            }
            // Compatible neighbors is now populated with props that are compatible with our added prop, but not each other
            std::vector<GlobalPropId> novelTuple;
            
            if (!novelTupleRuledOut)
            {
                for (auto ptid_it = compatibleNeighbors.begin(); ptid_it != compatibleNeighbors.end(); ++ptid_it)
                {
//                     std::cout << "Check for compatible neighbors" << std::endl;
                    if ((*ptid_it).first != propTypeId) // is prop of other type
                    {
                        // TODO Below needs to be updated to not pick the last available always once compatability is implemented!
                        PropIdRaw chosenProp = (*ptid_it).second.back(); // Just pick the last available compatible one
                        novelTuple.push_back({(*ptid_it).first, chosenProp});
//                         std::cout << "Add to novel tuple" << std::endl;
                        // Need to remove this from potentialNeighbors if it should not be part of multiple tuples of this typeset
                        // TODO: How to determine if a neighbor should be allowed to be reused (and not consumed)
                        bool isPartialStatic = false;
                        if (!isPartialStatic)
                        {
                            // We should only remove props if a novel tuple is formed
                            potentialNeighbors[(*ptid_it).first].pop_back();
                        }
                    }
                }
                for (auto& sunId : typesetSunLambdas[(*typeset_it)])
                {
//                     std::cout << "Formed novel tuple of " << SunLambdaRegistry::GetInstance().Get(sunId).name << std::endl;
                }
                
                // The prop that we're adding should be included in the novel tuple! Hooray!
                novelTuple.push_back({propTypeId, id});
                // The prop that we're adding should be removed from potential neighbors
//                 std::cout << "Inserting novel tuple of " << novelTuple.size() << " size" << std::endl;
                novelTuples[(*typeset_it)].push_back(novelTuple);
                
                // This could be optimized with an emerge only sunLambdaTypesets data structure
                for (auto emergesun_it = emerges.begin(); emergesun_it != emerges.end(); ++emergesun_it)
                {
                    if (sunLambdaTypesets[(*emergesun_it)] == (*typeset_it))
                    {
                        // Why are you doing this below lol, this is calling iterate props
//                         SunLambdaRegistry::GetInstance().Get((*emergesun_it))();
                        std::vector<PropIdRaw> sunData(novelTuple.size()); // The SunLambda already knows the types in order, therefore we only need pass it the PropIdRaw values and it can imply the types
                        for (GlobalPropId& gpid : novelTuple)
                            sunData.push_back(gpid.id);
                        SunLambda& sun = SunLambdaRegistry::GetInstance().Get(*(emergesun_it));
                        sun.Emerge(sunData);
                        // TODO Use constraints instead of consuming the emerge, probably
                        emerges.erase(emergesun_it); // Consume emerge!?
                        break;
                    }
                }
                
            } 
            else
            {
                BicycleMango::stagingPropTuples[(*typeset_it)][propTypeId].push_back(id);
            }
        }

        return &prop;
    }


    // Props should only be removed by stage rather than considering type
    static void RemovePropsOnStage(Stage stage)
    {
    }

    template <typename PropType>
    static PropType& GetProp(PropId<PropType> id)
    {
        return GetProps[id.id];
    }

    template <typename PropType>
    static HoleVector<PropType>& GetProps()
    {
        static HoleVector<PropType> props;
        return props;
    }

    template <typename PropType>
    static PropType* GetProp()
    {
        for(auto& prop : GetProps<PropType>())
        {
            return &prop;
        }

        return nullptr;
    }

private:

    template <typename ... PTypes, std::size_t ... Is>
    static auto IterateProps(void (*functor)(PTypes...), const SunLambda::Id& id, std::index_sequence<Is...> seq)
    {
        for (auto& tuple : novelTuples[sunLambdaTypesets[id]])
        {
            functor(GetProps<std::decay_t<PTypes>>()[tuple[Is].id]...);
        }
    }

public:

    template <typename ... PTypes>
    static auto IterateProps(void (*functor)(PTypes...), const SunLambda::Id& id)
    {
        return IterateProps<PTypes...>(functor, id, std::index_sequence_for<PTypes...> {});
    }
};



