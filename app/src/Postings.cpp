#include "Postings.h"

#include <utility>


Postings::Postings(Barracks barracks, TrainingCamp camp)
: barracks_(std::move(barracks)), camp_(std::move(camp))
{}

const Barracks &Postings::barracks() const
{
    return barracks_;
}

const TrainingCamp &Postings::camp() const
{
    return camp_;
}

Team &Postings::party(int partyIndex) 
{
    return barracks_.at(partyIndex);    
}

int Postings::createParty(const std::string &name)
{
    return barracks_.create(name);
}

void Postings::disband(int partyIndex)
{
    barracks_.disband(partyIndex);
}

void Postings::removeFromParty(int partyIndex, int unitId)
{
    party(partyIndex).removeMember(unitId);
}

void Postings::removeTrainer(int trainerId)
{
    camp_.removeTrainer(trainerId);
}

void Postings::removeTrainee(int traineeId)
{
    camp_.removeTrainee(traineeId);
}

void Postings::buySlot()
{
    camp_.buySlot();
}

void Postings::tickCamp(Roster &roster, const std::vector<Injury> &injuries,
                        std::mt19937 &rng)
{
    camp_.tick(roster, injuries, rng);
}

void Postings::purgeDead(const Roster &roster)
{
    barracks_.purgeDead(roster);
    camp_.purgeDead(roster);
}

void Postings::addTrainer(int unitId, const Roster &roster)
{
    camp_.addTrainer(unitId, roster);
    barracks_.release(unitId);
}

void Postings::assignTrainee(int trainerId, int traineeId, const Roster &roster)
{
    camp_.assignTrainee(trainerId, traineeId, roster);
    barracks_.release(traineeId);
}

void Postings::assignToParty(int unitId, int partyIndex, const Roster &roster)
{
    barracks_.assign(unitId, partyIndex, roster);
    camp_.removeTrainee(unitId);
    camp_.removeTrainer(unitId);
}

Postings Postings::fromSave(Barracks barracks, TrainingCamp camp, const GameState &state,
                          std::vector<Moved> &moved)
{
    Postings p(std::move(barracks), std::move(camp));

    for (int i = 0; i < p.barracks_.count(); i++)
    {
        bool away = false;
        for (const Sortie &s : state.sorties)
            if (s.partyIndex == i)
                away = true;
        
        std::vector<int> members = p.barracks_.at(i).getMembersIds();
        for (int id : members)
        {
            if (!p.camp_.contains(id))
                continue;
            
            Moved m;
            m.unitId = id;
            m.partyIndex = i;
            if (away)
            {
                m.kept = Moved::Kept::Party;
                for (const TrainingCamp::Assignment &a : p.camp_.assignments())
                if (a.trainerId == id)
                    m.freedTrainees = a.traineeIds;
                p.removeTrainer(m.unitId);
                p.removeTrainee(m.unitId);
            }
            else
            {
                m.kept = Moved::Kept::Camp;
                p.removeFromParty(m.partyIndex, m.unitId);
            }
            moved.push_back(m);
        }
    }
    return p;
}