/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/PlayerData/DS3_PlayerDataManager.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS3_GameIds.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"
#include "Shared/Core/Utils/PlayerDataUtils.h"

#include "Shared/Core/Network/NetConnection.h"

DS3_PlayerDataManager::DS3_PlayerDataManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_PlayerDataManager::OnMessageReceived(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdateLoginPlayerCharacter))
    {
        return Handle_RequestUpdateLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdatePlayerStatus))
    {
        return Handle_RequestUpdatePlayerStatus(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdatePlayerCharacter))
    {
        return Handle_RequestUpdatePlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetPlayerCharacter))
    {
        return Handle_RequestGetPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetLoginPlayerCharacter))
    {
        return Handle_RequestGetLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetPlayerCharacterList))
    {
        return Handle_RequestGetPlayerCharacterList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter*)Message.Protobuf.get();

    std::shared_ptr<Character> Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
    if (!Character)
    {
        std::vector<uint8_t> Data;        
        if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
            return MessageHandleResult::Error;
        }

        Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
        Ensure(Character);
    }

    DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Request->character_id());

    DS3_Frpg2RequestMessage::QuickMatchRank* Rank = Response.mutable_quickmatch_brawl_rank();
    Rank->set_rank(Character->QuickMatchBrawlRank);
    Rank->set_xp(Character->QuickMatchBrawlXp);

    Rank = Response.mutable_quickmatch_dual_rank();
    Rank->set_rank(Character->QuickMatchDuelRank);
    Rank->set_xp(Character->QuickMatchDuelXp);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (DS3_Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    auto& State = Client->GetPlayerStateType<DS3_PlayerState>();

    // Merge the delta into the current state.
    std::string bytes = Request->status();

    DS3_Frpg2PlayerData::AllStatus status;
    if (!status.ParseFromArray(bytes.data(), (int)bytes.size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse DS3_Frpg2PlayerData::AllStatus from RequestUpdatePlayerStatus.");

        // Don't take this as an error, it will resolve itself on next send.
        return MessageHandleResult::Handled;
    }

    MergePlayerStatusDelta(State, status);

    // Update character id field if present.
    UpdateCharacterId(State);

    // Keep track of the player's character name and possibly rename the connection.
    RenameConnectionIfCharacterNameChanged(State, Client->shared_from_this());

    // Print a log and update area state.
    HandleAreaChange(State, Client->shared_from_this());
    // Update matchmaking-related state (invadable, levels).
    UpdateMatchmakingState(State);

    // Determine correct visitor pool via shared helper.
    auto NewVisitorPool = DetermineVisitorPool(State);
    if (NewVisitorPool != State.GetVisitorPool())
    {
        State.SetVisitorPool(NewVisitorPool);
    }

#if 0//def _DEBUG
    static DiffTracker Tracker;
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_2", State.GetPlayerStatus().player_status().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_6", State.GetPlayerStatus().player_status().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_9", State.GetPlayerStatus().player_status().unknown_9());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_14", State.GetPlayerStatus().player_status().unknown_14());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_32", State.GetPlayerStatus().player_status().unknown_32());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_33", State.GetPlayerStatus().player_status().unknown_33());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_63", State.GetPlayerStatus().player_status().unknown_63());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_76", State.GetPlayerStatus().player_status().unknown_76());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_78", State.GetPlayerStatus().player_status().unknown_78());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_80", State.GetPlayerStatus().player_status().unknown_80());
    for (int i = 0; i < State.GetPlayerStatus().player_status().anticheat_data_size(); i++)
    {
        Tracker.Field(State.GetCharacterName().c_str(), StringFormat("PlayerStatus.player_status.anticheat[%i]", i), State.GetPlayerStatus().player_status().anticheat_data(i));
    }
    for (int i = 0; i < State.GetPlayerStatus().player_status().unknown_18_size(); i++)
    {
        Tracker.Field(State.GetCharacterName().c_str(), StringFormat("PlayerStatus.player_status.unknown_18[%i]", i), State.GetPlayerStatus().player_status().unknown_18(i));
    }
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.equipment.unknown_id_59", State.GetPlayerStatus().equipment().unknown_59());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.equipment.unknown_id_60", State.GetPlayerStatus().equipment().unknown_60());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_1", State.GetPlayerStatus().stats_info().unknown_1());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_2", State.GetPlayerStatus().stats_info().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_3", State.GetPlayerStatus().stats_info().unknown_3());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_4", State.GetPlayerStatus().stats_info().unknown_4());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_5", State.GetPlayerStatus().stats_info().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_6", State.GetPlayerStatus().stats_info().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.play_data.unknown_id_4", State.GetPlayerStatus().play_data().unknown_4());
#endif

    DS3_Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.");
        return MessageHandleResult::Error;
    }

    // Process any bonfires that were lit since last status.
    ProcessBonfires(State, ServerInstance, Client->shared_from_this());
    State.SetHasInitialState(true);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdatePlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->character_data().data(), Request->character_data().data() + Request->character_data().size());

    if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
        return MessageHandleResult::Error;
    }

    DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestGetPlayerCharacter*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterResponse Response;

    std::vector<uint8_t> CharacterData;
    std::shared_ptr<Character> Character = Database.FindCharacter(Request->player_id(), Request->character_id());
    if (Character)
    {
        CharacterData = Character->Data;
    }

    Response.set_player_id(Request->player_id());
    Response.set_character_id(Request->character_id());
    Response.set_character_data(CharacterData.data(), CharacterData.size());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetPlayerCharacterList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterList* Request = (DS3_Frpg2RequestMessage::RequestGetPlayerCharacterList*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_PlayerDataManager::GetName()
{
    return "Player Data";
}
