#include "olympics24a2.h"
#include <iostream>

olympics_t::olympics_t()
{
    //m_timestamp = 0;
    m_highestRank = -1;
	// TODO: Your code goes here
}

olympics_t::~olympics_t()
{
	// TODO: Your code goes here
}


StatusType olympics_t::add_team(int teamId)
{
    if(teamId <= 0)
    {
        return StatusType::INVALID_INPUT;
    }
    if(m_teamsHash.find(teamId) != nullptr)
    {
        return StatusType::FAILURE;
    }
    try
    {
        TeamByID* newTeam = new TeamByID(teamId);
        m_teamsHash.insert(teamId, newTeam);
        if(m_highestRank == -1)
        {
            m_highestRank = 0;
        }
    }catch(std::bad_alloc&)
    {
        return StatusType::ALLOCATION_ERROR;
    }
	return StatusType::SUCCESS;
}

StatusType olympics_t::remove_team(int teamId)
{
	if(teamId <= 0)
    {
        return StatusType::INVALID_INPUT;
    }
    if(m_teamsHash.find(teamId) == nullptr)
    {
        return StatusType::FAILURE;
    }
    m_teamsHash.remove(teamId);
    TeamByID teamIDFinder(teamId);
    Node<TeamByID>* team = m_teamsByID.findNode(&teamIDFinder);
    if(team != nullptr)
    {
        TeamByPower teamPowerFinder(teamId, team->m_info->m_wins, team->m_info->m_power);
        m_teamsByID.removeNode(&teamIDFinder);
        m_teamsByPower.removeNode(&teamPowerFinder);
    }
    if(m_teamsHash.m_occupancy == 0)
    {
        m_highestRank = -1;
    }
    else if(m_teamsByID.m_treeSize == 0)
    {
        m_highestRank = 0;
    }
    else
    {
        m_highestRank = m_teamsByPower.m_root->m_maxRank;
    }
	return StatusType::SUCCESS;
}

StatusType olympics_t::add_player(int teamId, int playerStrength)
{
    if(teamId <= 0 || playerStrength <= 0)
    {
        return StatusType::INVALID_INPUT;
    }
    TeamByID *teamInHash = m_teamsHash.find(teamId);
    if(teamInHash == nullptr)
    {
        return StatusType::FAILURE;
    }
    TeamByID teamFinder(teamId);
    if(m_teamsByID.findNode(&teamFinder) == nullptr)
    {
        try
        {
            TeamByID* newteam = new TeamByID(teamId, teamInHash->m_wins, playerStrength);
            m_teamsByID.insertNode(newteam);
        }catch(std::bad_alloc&)
        {
            return StatusType::ALLOCATION_ERROR;
        }
    }
    Node<TeamByID>* team = m_teamsByID.findNode(&teamFinder);
    try
    {
        PlayerByCreated *playerbycreated = new PlayerByCreated(team->m_info->m_player_count, playerStrength);
        team->m_info->m_playersByCreated->insertNode(playerbycreated);
        PlayerByStrength *playerbystrength = new PlayerByStrength(team->m_info->m_player_count, playerStrength);
        team->m_info->m_playersByStrength->insertNode(playerbystrength);
    }catch(std::bad_alloc&)
    {
        return StatusType::ALLOCATION_ERROR;
    }
    team->m_info->m_player_count++;
    TeamByPower teamPowerFinder(teamId, team->m_info->m_wins, team->m_info->m_power);
    Node<TeamByPower> *teampow = m_teamsByPower.findNode(&teamPowerFinder);
    int original_wins;
    if(teampow == nullptr)
    {
        original_wins = teamInHash->m_wins;
    }
    else
    {
        original_wins = teampow->m_info->m_wins + m_teamsByPower.getAddedWins(teampow->m_info);
    }
    m_teamsByPower.removeNode(&teamPowerFinder);
    team->m_info->updateMedianAndPower();
    TeamByPower* updatedTeamPower = new TeamByPower(teamId, original_wins, team->m_info->m_power);
    m_teamsByPower.insertNode(updatedTeamPower);
    Node<TeamByPower>* temp =  m_teamsByPower.findNode(updatedTeamPower);
    temp->m_maxRank = updatedTeamPower->m_wins + updatedTeamPower->m_power + m_teamsByPower.getAddedWins(temp->m_info);
    m_teamsByPower.updateMaxRec(temp);
    m_highestRank = m_teamsByPower.m_root->m_maxRank;
	return StatusType::SUCCESS;
}

StatusType olympics_t::remove_newest_player(int teamId)
{
	if(teamId <= 0)
    {
        return StatusType::INVALID_INPUT;
    }
    TeamByID teamFinder(teamId);
    Node<TeamByID>* team = m_teamsByID.findNode(&teamFinder);
    if(team == nullptr)
    {
        return StatusType::FAILURE;
    }
    PlayerByCreated* newest = maxNode(team->m_info->m_playersByCreated->m_root)->m_info;
    PlayerByStrength strcopy(newest->m_created, newest->m_strength);
    team->m_info->m_playersByCreated->removeNode(newest);
    team->m_info->m_playersByStrength->removeNode(&strcopy);
    team->m_info->m_player_count--;
    TeamByPower teamPowerFinder(teamId, team->m_info->m_wins, team->m_info->m_power);
    if(team->m_info->m_playersByCreated->m_treeSize == 0)
    {
        int added_wins = m_teamsByPower.getAddedWins(&teamPowerFinder);
        m_teamsHash.find(teamId)->m_wins = team->m_info->m_wins + added_wins;
        m_teamsByID.removeNode(team->m_info);
        m_teamsByPower.removeNode(&teamPowerFinder);
    }
    else
    {
        int original_wins;
        TeamByPower* teampow = m_teamsByPower.findNode(&teamPowerFinder)->m_info;
        original_wins = teampow->m_wins + m_teamsByPower.getAddedWins(teampow);
        m_teamsByPower.removeNode(&teamPowerFinder);
        team->m_info->updateMedianAndPower();
        TeamByPower *updatedPower = new TeamByPower(teamId, original_wins, team->m_info->m_power);
        m_teamsByPower.insertNode(updatedPower);
        Node<TeamByPower>* temp =  m_teamsByPower.findNode(updatedPower);
        temp->m_maxRank = updatedPower->m_wins + updatedPower->m_power + m_teamsByPower.getAddedWins(temp->m_info); //TODO is temp always a leaf?
        m_teamsByPower.updateMaxRec(temp);
    }
    if(m_teamsByID.m_treeSize == 0)
    {
        m_highestRank = 0;
    }
    else
    {
        m_highestRank = m_teamsByPower.m_root->m_maxRank;
    }
	return StatusType::SUCCESS;

    //TODO update median and power of rankteam
}

output_t<int> olympics_t::play_match(int teamId1, int teamId2)
{
    if(teamId1 <= 0 || teamId2 <= 0 || teamId1 == teamId2)
    {
        return StatusType::INVALID_INPUT;
    }
    TeamByID teamFinder1(teamId1);
    TeamByID teamFinder2(teamId2);
    Node<TeamByID>* team1 = m_teamsByID.findNode(&teamFinder1);
    Node<TeamByID>* team2 = m_teamsByID.findNode(&teamFinder2);
    if(team1 == nullptr || team2 == nullptr)
    {
        return StatusType::FAILURE;
    }
    TeamByPower teamPowerFinder1(teamId1, team1->m_info->m_wins, team1->m_info->m_power);
    TeamByPower teamPowerFinder2(teamId2, team2->m_info->m_wins, team2->m_info->m_power);
    TeamByPower *team1Power = m_teamsByPower.findNode(&teamPowerFinder1)->m_info;
    TeamByPower *team2Power = m_teamsByPower.findNode(&teamPowerFinder2)->m_info;
    int winningID;
    if(team1->m_info->m_power > team2->m_info->m_power)
    {
        team1->m_info->m_wins++;
        team1Power->m_wins++;
        winningID = teamId1;
        m_teamsHash.find(teamId1)->m_wins++;
    }
    else if(team1->m_info->m_power < team2->m_info->m_power)
    {
        team2->m_info->m_wins++;
        team2Power->m_wins++;
        winningID = teamId2;
        m_teamsHash.find(teamId2)->m_wins++;
    }
    else
    {
        if(teamId1 < teamId2)
        {
            team1->m_info->m_wins++;
            team1Power->m_wins++;
            winningID = teamId1;
            m_teamsHash.find(teamId1)->m_wins++;
        }
        else
        {
            team2->m_info->m_wins++;
            team2Power->m_wins++;
            winningID = teamId2;
            m_teamsHash.find(teamId2)->m_wins++;
        }
    }
    Node<TeamByPower>* temp1 =  m_teamsByPower.findNode(team1Power);
    temp1->m_maxRank = temp1->m_info->m_wins + temp1->m_info->m_power + m_teamsByPower.getAddedWins(temp1->m_info);
    m_teamsByPower.updateMaxRec(temp1);
    Node<TeamByPower>* temp2 =  m_teamsByPower.findNode(team2Power);
    temp2->m_maxRank = temp2->m_info->m_wins + temp2->m_info->m_power + m_teamsByPower.getAddedWins(temp2->m_info);
    m_teamsByPower.updateMaxRec(temp2);
    m_highestRank = m_teamsByPower.m_root->m_maxRank;
    return winningID;
}

output_t<int> olympics_t::num_wins_for_team(int teamId)
{
    if(teamId <= 0)
    {
        return StatusType::INVALID_INPUT;
    }
    if(m_teamsHash.find(teamId) == nullptr)
    {
        return StatusType::FAILURE;
    }
    TeamByID teamIDFinder(teamId);
    Node<TeamByID> *team = m_teamsByID.findNode(&teamIDFinder);
    if(team == nullptr)
    {
        return m_teamsHash.find(teamId)->m_wins;
    }
    TeamByPower teamPowerFinder(teamId, team->m_info->m_wins, team->m_info->m_power);
    TeamByPower *teamByPower = m_teamsByPower.findNode(&teamPowerFinder)->m_info;
    return teamByPower->m_wins + m_teamsByPower.getAddedWins(teamByPower);
}

output_t<int> olympics_t::get_highest_ranked_team()
{
    //std::cout << "max power id: " << maxNode(m_teamsByPower.m_root)->m_info->m_teamID << '\n';
    //std::cout << "max power: " << maxNode(m_teamsByPower.m_root)->m_info->m_power << '\n';
    if(m_teamsHash.m_occupancy == 0)
    {
        return -1;
    }
    return m_highestRank;
}

StatusType olympics_t::unite_teams(int teamId1, int teamId2)
{
	if(teamId1 <= 0 || teamId2 <= 0 || teamId1 == teamId2)
    {
        return StatusType::INVALID_INPUT;
    }
    TeamByID teamFinder1(teamId1);
    TeamByID teamFinder2(teamId2);
    TeamByID *team1hash = m_teamsHash.find(teamId1);
    TeamByID *team2hash = m_teamsHash.find(teamId2);
    if(team1hash == nullptr || team2hash == nullptr)
    {
        return StatusType::FAILURE;
    }
    Node<TeamByID>* team1 = m_teamsByID.findNode(&teamFinder1);
    Node<TeamByID>* team2 = m_teamsByID.findNode(&teamFinder2);
    if(team2 == nullptr)
    {
        m_teamsHash.remove(teamId2);
        return StatusType::SUCCESS;
    }
    else if(team1 == nullptr)
    {
        TeamByID *team1new = new TeamByID(teamId1, team1hash->m_wins, team1hash->m_power);
        m_teamsByID.insertNode(team1new);
        team1 = m_teamsByID.findNode(team1new);
    }
    TeamByPower team1PowerFinder(teamId1, team1->m_info->m_wins, team1->m_info->m_power);
    TeamByPower team2PowerFinder(teamId2, team2->m_info->m_wins, team2->m_info->m_power);
    Node<TeamByPower> *team1pow = m_teamsByPower.findNode(&team1PowerFinder);
    int original_wins;
    if(team1pow == nullptr)
    {
        original_wins = team1hash->m_wins;
    }
    else
    {
        original_wins = team1pow->m_info->m_wins + m_teamsByPower.getAddedWins(team1pow->m_info);
    }
    Node<TeamByPower> *team2pow = m_teamsByPower.findNode(&team2PowerFinder);
    m_teamsByPower.removeNode(&team1PowerFinder);
    m_teamsByPower.removeNode(&team2PowerFinder);
    int sizeOfArray1 = 0;
    if(team1 != nullptr)
    {
        sizeOfArray1 = team1->m_info->m_playersByCreated->m_treeSize;
    }
    int sizeOfArray2 = team2->m_info->m_playersByCreated->m_treeSize;

    PlayerByCreated **arrayCreated1;
    PlayerByCreated **arrayCreated2;
    PlayerByCreated **arrayMergedCreated;
    PlayerByStrength **arrayStrength1;
    PlayerByStrength **arrayStrength2;
    PlayerByStrength **arrayMergedStrength;

    try
    {
        arrayCreated1 = new PlayerByCreated *[sizeOfArray1];
        arrayCreated2 = new PlayerByCreated *[sizeOfArray2];
        arrayMergedCreated = new PlayerByCreated *[sizeOfArray1+sizeOfArray2];
        arrayStrength1 = new PlayerByStrength *[sizeOfArray1];
        arrayStrength2 = new PlayerByStrength *[sizeOfArray2];
        arrayMergedStrength = new PlayerByStrength *[sizeOfArray1+sizeOfArray2];
    } catch (std::bad_alloc &)
    {
        return StatusType::ALLOCATION_ERROR;
    }
    InorderTransversalIntoArray(team1->m_info->m_playersByCreated->m_root, arrayCreated1, sizeOfArray1, 0);
    InorderTransversalIntoArray(team2->m_info->m_playersByCreated->m_root, arrayCreated2, sizeOfArray2, 0);
    InorderTransversalIntoArray(team1->m_info->m_playersByStrength->m_root, arrayStrength1, sizeOfArray1, 0);
    InorderTransversalIntoArray(team2->m_info->m_playersByStrength->m_root, arrayStrength2, sizeOfArray2, 0);
    int inc;
    if(team1->m_info->m_playersByCreated->m_treeSize == 0)
    {
        inc = 0;
    }
    else
    {
        inc = team1->m_info->m_player_count;
                // maxNode(team1->m_info->m_playersByCreated->m_root)->m_info->m_created;
    }
    for(int i = 0; i < sizeOfArray2; i++)
    {
        arrayCreated2[i]->m_created += inc;
        arrayStrength2[i]->m_created += inc;
    }
    team1->m_info->m_player_count += inc;
    mergeTwoArraysIntoOne(arrayCreated1, arrayCreated2, arrayMergedCreated, sizeOfArray1, sizeOfArray2);
    mergeTwoArraysIntoOne(arrayStrength1, arrayStrength2, arrayMergedStrength, sizeOfArray1, sizeOfArray2);

    Node<PlayerByCreated> *newRootCreated = mergedArrayIntoBalTree(arrayMergedCreated, 0, sizeOfArray1 + sizeOfArray2 - 1);
    Node<PlayerByStrength> *newRootStrength = mergedArrayIntoBalTree(arrayMergedStrength, 0, sizeOfArray1 + sizeOfArray2 - 1);

    //delete team1->m_info->m_playersByCreated;
    //delete team1->m_info->m_playersByStrength;

    team1->m_info->m_playersByCreated = new AVLRankTree<PlayerByCreated>();
    team1->m_info->m_playersByStrength = new AVLRankTree<PlayerByStrength>();
    team1->m_info->m_playersByCreated->m_root = newRootCreated;
    team1->m_info->m_playersByStrength->m_root = newRootStrength;
    team1->m_info->m_playersByCreated->m_treeSize = sizeOfArray1 + sizeOfArray2;
    team1->m_info->m_playersByStrength->m_treeSize = sizeOfArray1 + sizeOfArray2;
    team2->m_info->m_playersByCreated->m_root = nullptr;
    team2->m_info->m_playersByStrength->m_root = nullptr;
    m_teamsByID.removeNode(team2->m_info);
    team1->m_info->updateMedianAndPower();
    TeamByPower *updatedTeamPower1 = new TeamByPower(team1->m_info->m_teamID, original_wins, team1->m_info->m_power);
    m_teamsByPower.insertNode(updatedTeamPower1);
    m_teamsHash.remove(teamId2);
    m_highestRank = m_teamsByPower.m_root->m_maxRank;
    return StatusType::SUCCESS;
}

output_t<int> olympics_t::play_tournament(int lowPower, int highPower)
{
    if(lowPower <= 0 || highPower <= 0 || highPower <= lowPower)
    {
        return StatusType::INVALID_INPUT;
    }
    if(m_teamsByID.m_treeSize == 0)
    {
        return StatusType::FAILURE;
    }
    TeamByID *maxIDTeam = maxNode(m_teamsByID.m_root)->m_info;
    TeamByPower *lowestInRange;
    TeamByPower *highestInRange;
    TeamByPower *lowPowerTeam = new TeamByPower(maxIDTeam->m_teamID + 1,0,lowPower);
    m_teamsByPower.insertNode(lowPowerTeam);
    Node<TeamByPower> * lowernode = m_teamsByPower.findNode(lowPowerTeam);

    if (lowernode->m_left != nullptr && lowernode->m_right != nullptr) //if has two sons
    {
        lowestInRange = minNode(lowernode->m_right)->m_info;
    }

    else if ((lowernode->m_left == nullptr && lowernode->m_right != nullptr)) //if only has right son todo check
    {
        lowestInRange = minNode(lowernode->m_right)->m_info;
    }

    else if (lowernode->m_left != nullptr && lowernode->m_right == nullptr) //if only has left son
    {
        if (*(maxNode(m_teamsByPower.m_root)->m_info) == lowernode->m_info)
        {
            lowestInRange = nullptr;
        }
        else if(lowernode->m_parent == nullptr)
        {
            lowestInRange = nullptr;
        }
        else if(lowernode->m_parent->m_right == lowernode)
        {
            Node<TeamByPower> *ptr = lowernode;

            while (ptr->m_parent != nullptr && ptr->m_parent->m_right == ptr)
            {
                ptr = ptr->m_parent;
            }
            if (ptr->m_parent != nullptr)
            {
                lowestInRange = ptr->m_parent->m_info;
            } else
            {
                lowestInRange = nullptr;
            }
        }else if (lowernode->m_parent->m_left == lowernode)
        {
            lowestInRange = lowernode->m_parent->m_info;
        }
    }else
    {
        if (lowernode->m_parent == nullptr) //if player is a leaf (no sons)
        {
            lowestInRange = nullptr;
        } else if (lowernode->m_parent->m_left == lowernode)
        {
            lowestInRange = lowernode->m_parent->m_info;
        } else if (lowernode->m_parent->m_right == lowernode)
        {

            Node<TeamByPower> *ptr = lowernode;

            while (ptr->m_parent != nullptr && ptr->m_parent->m_right == ptr)
            {
                ptr = ptr->m_parent;
            }
            if (ptr->m_parent != nullptr)
            {
                lowestInRange = ptr->m_parent->m_info;
            } else
            {
                lowestInRange = nullptr;
            }
        }
    }
    m_teamsByPower.removeNode(lowPowerTeam);
    TeamByPower *highPowerTeam = new TeamByPower(-1, 0, highPower);
    m_teamsByPower.insertNode(highPowerTeam);
    Node<TeamByPower> *highernode = m_teamsByPower.findNode(highPowerTeam);

    if (highernode->m_left != nullptr && highernode->m_right != nullptr) //if has two sons
    {
        highestInRange = maxNode(highernode->m_left)->m_info;

    }

    else if ((highernode->m_left == nullptr && highernode->m_right != nullptr)) //if only has right son todo check
    {
        if (*(minNode(m_teamsByPower.m_root)->m_info) == highernode->m_info)
        {
            highestInRange = nullptr;
        }
        else if(highernode->m_parent == nullptr)
        {
            highestInRange = nullptr;
        }
        else if(highernode->m_parent->m_left == highernode)
        {
            Node<TeamByPower> *ptr = highernode;

            while (ptr->m_parent != nullptr && ptr->m_parent->m_left == ptr)
            {
                ptr = ptr->m_parent;
            }
            if (ptr->m_parent != nullptr)
            {
                highestInRange = ptr->m_parent->m_info;
            } else
            {
                highestInRange = nullptr;
            }
        }else if (highernode->m_parent->m_right == highernode)
        {
            highestInRange = highernode->m_parent->m_info;
        }
    }
    else
    {
        if (highernode->m_parent == nullptr) //if leaf
        {
            highestInRange = nullptr;
        } else if (highernode->m_parent->m_left == highernode)
        {
            Node<TeamByPower> *ptr = highernode;

            while (ptr->m_parent != nullptr && ptr->m_parent->m_left == ptr)
            {
                ptr = ptr->m_parent;
            }
            if (ptr->m_parent != nullptr)
            {
                highestInRange = ptr->m_parent->m_info;
            } else
            {
                highestInRange = nullptr;
            }
        }
        if (highernode->m_parent->m_right == highernode)
        {
            highestInRange = highernode->m_parent->m_info;
        }
    }
    m_teamsByPower.removeNode(highPowerTeam);
    if(lowestInRange == nullptr || highestInRange == nullptr)
    {
        return StatusType::FAILURE;
    }

    int lowrank = m_teamsByPower.rank(lowestInRange);
    int highrank = m_teamsByPower.rank(highestInRange);

    int num_in_range = highrank - lowrank + 1;
    int temp = num_in_range;
    if(temp <= 1)
    {
        return StatusType::FAILURE;
    }
    while(temp != 0)
    {
        if(temp % 2 != 0 && temp != 1)
        {
            return StatusType::FAILURE;
        }
        temp /=2;
    }
    int j = m_teamsByPower.rank(highestInRange);
    int i = m_teamsByPower.rank(lowestInRange);
    int middle;
    while(num_in_range != 2)
    {
        if (i % 2 == 1 && j % 2 == 0 || j % 2 == 1 && i % 2 == 0)
        {
            middle = i + (j - i) / 2 + 1;
        } else
        {
            middle = i + (j - i) / 2;
        }
        TeamByPower *middleTeam = m_teamsByPower.select(m_teamsByPower.m_root, middle - 1)->m_info;
        m_teamsByPower.addWinsToLessEqual(highestInRange, 1);
        m_teamsByPower.addWinsToLessEqual(middleTeam, -1);
        num_in_range /= 2;
        i = middle;
    }
    if(num_in_range == 2)
    {
        highestInRange->m_wins ++;
        TeamByID winnerFinder(highestInRange->m_teamID);
        TeamByID *winnerByID = m_teamsByID.findNode(&winnerFinder)->m_info;
        winnerByID->m_wins++;
        Node<TeamByPower> *highestnode = m_teamsByPower.findNode(highestInRange);
        highestnode->m_maxRank = highestInRange->m_power + highestInRange->m_wins + m_teamsByPower.getAddedWins(highestInRange);
        m_teamsByPower.updateMaxRec(highestnode);
    }
    m_highestRank = m_teamsByPower.m_root->m_maxRank;
    return highestInRange->m_teamID;
}