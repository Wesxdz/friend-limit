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
    // ALL STAGES OF A PROP MUST RETURN TRUE ON THE COMPATIBILITY CONSTRAINT FOR THAT PROP TO BE CONSIDERED TO FORM PART OF A NOVEL TUPLE OF THE SunLambda::Id WHICH HAS THIS CompatibleConstraint AS PART OF ITS NovelTupleCreator
    // Can this be considered to be part of a novel tuple?
    using CompatibleConstraint = std::function<bool(PropTypeId, const Stage&)>;
    // Can this prop be reused in novel tuples
    using PartialStaticIndicators = std::unordered_map<PropTypeId, std::function<bool(const Stage&)>>;
    
    struct NovelTupleCreator
    {
        CompatibleConstraint compatible;
        PartialStaticIndicators stageConstraints;
    };

    static inline std::set<SunLambda::Id> emerges;
    static void Emerge(SunLambda::Id id, CompatibleConstraint compatible = All, PartialStaticIndicators stageConstraints = {})
    {
        emerges.insert(id);
        novelTupleCreators[id] = {compatible, stageConstraints};
    }
    
    // Shared between emerge/plan/breakup: we assume a SunLambda cannot have multiple NovelTupleCreators for now
    static inline std::unordered_map<SunLambda::Id, NovelTupleCreator> novelTupleCreators;
    // This data structure is an augment of potential neighbors when searching for props to form novel tuples
    static inline std::unordered_map<SunLambda::Id, std::vector<std::vector<PropIdRaw>>> partialStatics;
    
    static inline CompatibleConstraint All = [](PropTypeId, const Stage&){return true;};
    static void Plan(SunLambda::Id id, const ScheduleSpecificity& specificity, CompatibleConstraint compatible = All, PartialStaticIndicators stageConstraints = {})
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
    static void Breakup(SunLambda::Id id, CompatibleConstraint compatible = All, PartialStaticIndicators stageConstraints = {})
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

    static inline std::unordered_map<PropTypeId, std::unordered_map<PropIdRaw, std::set<Stage, InstanceComparator>>> ptpsq; // Prop type prop stages query
    // SHOULD WE ONLY STORE THIS INFORMATION FOR PARTIAL STATICS OR EVERYTHING???
//     static inline std::unordered_map<PropTypeId, std::unordered_map<Group, std::unordered_map<Instance, std::set<PropIdRaw>>>> ptgid;
    
    struct GlobalPropId {PropTypeId typeId;PropIdRaw id;};
    friend inline bool operator< (const GlobalPropId& lhs, const GlobalPropId& rhs){ return lhs.typeId < rhs.typeId && lhs.id < rhs.id; }


    // Rather ironically, Typeset is a vector because the prop types must be delivered in a certain order to the SunLambda functor despite the conceptual set of props being acted upon
    using Typeset = std::vector<PropTypeId>;
    // All SunLambda and Jolt typeset signatures used (we should only keep track of novel tuples of typesets that are used by gameplay programmers by adding considered typesets in the DefineSunLambda constructor)
    
    static inline std::unordered_map<PropTypeId, std::set<Typeset>> mappedPropTupleTypesets;
    static inline std::set<Typeset> globalPropTupleTypesets;

    // Each typeset stores a map of types to props which have been added and have not been formed into novel tuples of this SunLambda typeset yet
    // Partial static indicators that return true
    static inline std::map<SunLambda::Id, std::unordered_map<PropTypeId, std::vector<PropIdRaw>>> stagingPropTuples;
    
    static inline std::map<SunLambda::Id, std::vector<std::vector<GlobalPropId>>> novelTuples;
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
        ptpsq[GetPropTypeId<PropType>()][propId.id].insert(stage);
//         ptgid[GetPropTypeId<PropType>()][stage.group][stage.instance].insert(propId.id);
    }

    template<typename PropType>
    static void AddPropStages(PropId<PropType> propId, const GroupSet& stages)
    {
        for(const Stage& stage : stages)
        {
            AddPropStage(propId, stage);
        }
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
        
        // ---
        for (auto typeset_it = typesetsWithAddedPropType.begin(); typeset_it != typesetsWithAddedPropType.end(); ++typeset_it)
        {
            for (auto sunlambda_it = typesetSunLambdas[(*typeset_it)].begin();
                 sunlambda_it != typesetSunLambdas[(*typeset_it)].end(); 
                ++sunlambda_it)
            {
        // --- 'for each SunLambda that PropType is a parameter of'
                
                // Check if a novel tuple is formed with this prop from the staged prop neighbors of each SunLambda that propTypeId is in!
                // Partial statics are not considered as potentialNeighbors because then we'd have to copy the stagingPropTuples vector rather than using a ref
                std::unordered_map<PropTypeId, std::vector<PropIdRaw>>& potentialNeighbors = BicycleMango::stagingPropTuples[(*sunlambda_it)];
                // However, both potential neighbors AND partial statics will be added to compatibleNeighbors later in this function
                std::unordered_map<PropTypeId, std::vector<PropIdRaw>> compatibleNeighbors;
                bool addedPropFulfillsCompatabilityConstraint = true;
                bool novelTupleRuledOut = false;
                
                auto IsPropCompatibleWithSunLambda = [sunlambda_it](PropTypeId ptid, PropIdRaw rid) -> bool
                {
                    std::cout << "ptid: " << ptid << ", " << "id: " << rid << " on SunLambda: " << SunLambdaRegistry::GetInstance().Get((*sunlambda_it)).name << std::endl;
                    // TODO Emerges cause bad_function_call
                    for (const Stage& stage : ptpsq[ptid][rid])
                    {
                        if (novelTupleCreators.count((*sunlambda_it)) == 0)
                        {
                            std::cout << "Has no novel tuple creator!";
                        }
                        if (!novelTupleCreators[(*sunlambda_it)].compatible(ptid, stage))
                        {
                            return false;
                        }
                    }
                    return true;
                };
                
                // The first thing we need to check is if the prop we're adding fulfills the compatability constraint for this SunLambda, if not then we can short circut everything
                if (!IsPropCompatibleWithSunLambda(propTypeId, id))
                {
                    std::cout << "Prop does not fulfill compatability constraint!" << std::endl;
                    addedPropFulfillsCompatabilityConstraint = false;
                    novelTupleRuledOut = true;
                }
                
                bool isAddedPropPartialStatic = false;
                // TODO && addedProp is not a partial static
                if (novelTupleCreators.count((*sunlambda_it)) > 0)
                {
                    NovelTupleCreator& creator = novelTupleCreators[(*sunlambda_it)];
                    if (creator.stageConstraints.count(propTypeId) > 0)
                    {
                        std::cout << "Has stage constraints!" << std::endl;
                        auto& reuse = creator.stageConstraints[propTypeId];
                        for (const Stage& stage : ptpsq[propTypeId][id])
                        {
                            std::cout << "Test stage: " << stage << std::endl;
                            if (reuse(stage))
                            {
                                std::cout << "REUSE PROP!!!!" << std::endl;
                                isAddedPropPartialStatic = true;
                                size_t i = 0;
                                for (const PropTypeId& ptid : (*typeset_it))
                                {
                                    if (ptid == propTypeId)
                                    {
                                        // I really hope this automatically resizes lol
                                        std::vector<std::vector<PropIdRaw>>& sad = partialStatics[(*sunlambda_it)];
                                        if (sad.size() == 0)
                                        {
                                            std::vector<PropIdRaw> p((*typeset_it).size());
                                            sad.push_back(p);
                                        }
                                        std::cout << sad.size() << std::endl;
                                        sad[0][i] = id; // TODO replace 0, WHICH PARTIAL STATIC TO CHOOSE?? creator.stageConstraints
                                        
                                    }
                                    i++;
                                    break;
                                }
                                break;
                            }
                        }
                    }
                }
                
                auto FindPartialStatic = [sunlambda_it, compatibleNeighbors, IsPropCompatibleWithSunLambda](PropTypeId ptid) mutable
                {
                    for (auto& partialStaticVector : partialStatics[(*sunlambda_it)])
                    {
                        for (const PropIdRaw& partialStatic : partialStaticVector)
                        {
                            if (IsPropCompatibleWithSunLambda(ptid, partialStatic))
                            {
                                compatibleNeighbors[ptid].push_back(partialStatic);
                                std::cout << "Found partial static" << std::endl;
                                return true;
                            }
                        }
                    }
                    return false;
                };
                
                if (!novelTupleRuledOut)
                {
                    // We need to make sure that potentialNeighbors has at least one prop of each type in this SunLambda::Id other than the type of prop being added, otherwise we can rule out creating a novel tuple
                    for (PropTypeId ptid : (*typeset_it))
                    {
                        if (ptid != propTypeId && potentialNeighbors[ptid].empty())
                        {
                            if (!FindPartialStatic(ptid))
                            {
                                novelTupleRuledOut = true;
                                break;
                            }
                        }
                    }
                }
                
                if (!novelTupleRuledOut)
                {
                    for (auto ptid_it = potentialNeighbors.begin(); ptid_it != potentialNeighbors.end(); ++ptid_it)
                    {
                        if ((*ptid_it).first != propTypeId) // is prop type different than the one being added
                        {
                            for (auto neighborId : (*ptid_it).second)
                            {
                                if (IsPropCompatibleWithSunLambda((*ptid_it).first, neighborId))
                                {
                                    compatibleNeighbors[(*ptid_it).first].push_back(neighborId);
                                    break; // Not sure what to do if there are multiple compatible neighbors, for now we just FIFO after considering compatability
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
                // Compatible neighbors is now populated with props that are compatible with (*sunlambda_it)
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
                    std::cout << "Formed novel tuple of " << SunLambdaRegistry::GetInstance().Get((*sunlambda_it)).name << std::endl;
                    
                    // The prop that we're adding should be included in the novel tuple! Hooray!
                    novelTuple.push_back({propTypeId, id});
                    // The prop that we're adding should be removed from potential neighbors
    //                 std::cout << "Inserting novel tuple of " << novelTuple.size() << " size" << std::endl;
                    novelTuples[(*sunlambda_it)].push_back(novelTuple);
                    
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
                else if (addedPropFulfillsCompatabilityConstraint)
                {
                    if (!isAddedPropPartialStatic)
                    {
                        BicycleMango::stagingPropTuples[(*sunlambda_it)][propTypeId].push_back(id);
                    }
                }
            } // --- end sunlambda_it
        } // --- end typeset_it

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
        for (auto& tuple : novelTuples[id])
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



