#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "login.h"
#include "netbuf.h"
#include "nbt.h"
#include "protocol.h"
#include "varint.h"
#include "uuid.h"
#include "logger.h"
#include "codec_data.h"

// 构造一个最小化的 NBT 注册表，参考 UCraft 实现
void write_minimal_registry(netbuf *buf) {
    nbt_begin_root_compound(buf); // 根节点 (匿名)

    // --- 1. dimension_type ---
    // 注意：注册表键名不带 minecraft: 前缀
    nbt_begin_compound(buf, "dimension_type");
        nbt_write_string(buf, "type", "dimension_type");
        nbt_begin_list(buf, "value", TAG_Compound, 1);
            nbt_begin_compound(buf, NULL);
                nbt_write_string(buf, "name", "minecraft:overworld");
                nbt_write_int(buf, "id", 0);
                nbt_begin_compound(buf, "element");
                    nbt_write_byte(buf, "piglin_safe", 0);
                    nbt_write_int(buf, "monster_spawn_light_level", 15);
                    nbt_write_int(buf, "monster_spawn_block_light_limit", 0);
                    nbt_write_byte(buf, "natural", 1);
                    nbt_write_float(buf, "ambient_light", 0.0f);
                    nbt_write_string(buf, "infiniburn", "#minecraft:infiniburn_overworld");
                    nbt_write_byte(buf, "respawn_anchor_works", 0);
                    nbt_write_byte(buf, "has_skylight", 1);
                    nbt_write_byte(buf, "bed_works", 1);
                    nbt_write_string(buf, "effects", "minecraft:overworld");
                    nbt_write_byte(buf, "has_raids", 1);
                    nbt_write_int(buf, "logical_height", 384);
                    nbt_write_double(buf, "coordinate_scale", 1.0);
                    nbt_write_int(buf, "min_y", -64);
                    nbt_write_byte(buf, "ultrawarm", 0);
                    nbt_write_byte(buf, "has_ceiling", 0);
                    nbt_write_int(buf, "height", 384);
            nbt_end_compound(buf); // element end
        nbt_end_compound(buf); // list item end
    nbt_end_compound(buf); // dimension_type end

    // --- 2. worldgen/biome ---
    nbt_begin_compound(buf, "worldgen/biome");
        nbt_write_string(buf, "type", "worldgen/biome");
        nbt_begin_list(buf, "value", TAG_Compound, 1);
            nbt_begin_compound(buf, NULL);
                nbt_write_string(buf, "name", "minecraft:plains");
                nbt_write_int(buf, "id", 0);
                nbt_begin_compound(buf, "element");
                    nbt_write_byte(buf, "has_precipitation", 1);
                    nbt_write_float(buf, "depth", 0.125f);
                    nbt_write_float(buf, "temperature", 0.8f);
                    nbt_write_float(buf, "scale", 0.05f);
                    nbt_write_float(buf, "downfall", 0.4f);
                    nbt_write_string(buf, "category", "plains");
                    nbt_begin_compound(buf, "effects");
                        nbt_write_int(buf, "sky_color", 7907327);
                        nbt_write_int(buf, "water_fog_color", 329011);
                        nbt_write_int(buf, "water_color", 4159231);
                        nbt_write_int(buf, "fog_color", 12638463);
                        nbt_begin_compound(buf, "mood_sound");
                            nbt_write_double(buf, "offset", 2.0);
                            nbt_write_int(buf, "tick_delay", 6000);
                            nbt_write_string(buf, "sound", "ambient.cave");
                            nbt_write_int(buf, "block_search_extent", 8);
                        nbt_end_compound(buf); // mood_sound end
                    nbt_end_compound(buf); // effects end
                nbt_end_compound(buf); // element end
            nbt_end_compound(buf); // list item end
        nbt_end_compound(buf); // biome end

    // --- 3. chat_type ---
    nbt_begin_compound(buf, "chat_type");
        nbt_write_string(buf, "type", "chat_type");
        nbt_begin_list(buf, "value", TAG_Compound, 1);
            nbt_begin_compound(buf, NULL); // list item
                nbt_write_string(buf, "name", "minecraft:chat");
                nbt_write_int(buf, "id", 0);
                nbt_begin_compound(buf, "element");
                    nbt_begin_compound(buf, "chat");
                        nbt_write_string(buf, "translation_key", "chat.type.text");
                        nbt_begin_list(buf, "parameters", TAG_String, 2);
                            nbt_write_string_value(buf, "sender");
                            nbt_write_string_value(buf, "content");
                        nbt_end_list(buf);
                    nbt_end_compound(buf); // chat end
                    nbt_begin_compound(buf, "narration");
                        nbt_write_string(buf, "translation_key", "chat.type.text.narrate");
                        nbt_begin_list(buf, "parameters", TAG_String, 2);
                            nbt_write_string_value(buf, "sender");
                            nbt_write_string_value(buf, "content");
                        nbt_end_list(buf);
                    nbt_end_compound(buf); // narration end
                nbt_end_compound(buf); // element end
            nbt_end_compound(buf); // list item end
        nbt_end_compound(buf); // chat_type end

    // --- 4. damage_type ---
    nbt_begin_compound(buf, "damage_type");
        nbt_write_string(buf, "type", "damage_type");
        nbt_begin_list(buf, "value", TAG_Compound, 46);
            // 完整的 damage_type 列表 (1.20.1)
            const char *damage_types[] = {
                "in_fire", "lightning_bolt", "on_fire", "lava", "hot_floor",
                "in_wall", "cramming", "drown", "starve", "cactus",
                "fall", "fly_into_wall", "out_of_world", "generic",
                "magic", "wither", "dragon_breath", "dry_out",
                "sweet_berry_bush", "freeze", "falling_block", "sting",
                "mob_attack", "mob_attack_no_aggro", "player_attack",
                "arrow", "trident", "mob_projectile", "fireworks",
                "unattributed_fireball", "fireball", "wither_skull",
                "thrown", "indirect_magic", "thorns", "explosion",
                "player_explosion", "sonic_boom", "bad_respawn_point",
                "outside_border", "generic_kill", "out_of_border",
                "fell_out_of_world", "too_fast", "stalagmite",
                "stalactite"
            };
            for (int i = 0; i < 46; i++) {
                nbt_begin_compound(buf, NULL); // list item
                    nbt_write_string(buf, "name", damage_types[i]);
                    nbt_write_int(buf, "id", i);
                    nbt_begin_compound(buf, "element");
                        nbt_write_string(buf, "message_id", damage_types[i]);
                        nbt_write_string(buf, "scaling", "never");
                        nbt_write_float(buf, "exhaustion", 0.1f);
                    nbt_end_compound(buf); // element end
                nbt_end_compound(buf); // list item end
            }
        nbt_end_compound(buf); // damage_type end

    nbt_end_compound(buf); // 整个 Registry 根结束
}

void send_login_success(client_connection *conn) {
    /* Send Login Success (ID 0x02) */
    netbuf *resp = netbuf_new(1024);
    write_varint(resp, 0x02); 

    /* Generate offline UUID from player name (deterministic) */
    uuid_t uuid;
    memset(uuid, 0, 16);
    size_t name_len = strlen(conn->player_name);
    for (size_t i = 0; i < name_len && i < 16; i++) {
        uuid[i] = (unsigned char)conn->player_name[i];
    }
    /* Set version 4 and variant */
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x0f) | 0xa0;

    conn->uuid = malloc(16);
    memcpy(conn->uuid, uuid, 16);

    for(int i=0; i<16; i++) write_byte(resp, uuid[i]);

    /* Write player name */
    write_string(resp, conn->player_name);

    // 写入属性数量 (离线模式通常为 0)
    write_varint(resp, 0);

    netbuf_send(conn->socket, resp);
    netbuf_free(resp);
}

void send_login_play(client_connection *conn) {
    netbuf *p = netbuf_new(8192); // 这个包很大，给足空间

    // Packet ID: 0x28 (Login Play)
    write_varint(p, 0x28);

    // 1. Entity ID
    write_be32_to_buf(p, 100);

    // 2. Is Hardcore
    write_byte(p, 0);

    // 3. Gamemode
    write_byte(p, 0); // Survival

    // 4. Previous Gamemode (-1 = not set)
    write_byte(p, -1);

    // 5. Dimension Names (Array of Identifier)
    write_varint(p, 1);
    write_string(p, "minecraft:overworld");

    // 6. Registry Data - 使用参考项目的原始数据
    for (size_t i = 0; i < codec_nbt_len; i++) {
        write_byte(p, codec_nbt[i]);
    }

    // 7. Dimension Type & Dimension Name
    write_string(p, "minecraft:overworld");
    write_string(p, "minecraft:overworld");

    // 8. Hashed Seed
    write_long_to_buf(p, 12345678L);

    // 9. Max Players (VarInt)
    write_varint(p, 20);

    // 10. View Distance & Simulation Distance (VarInt)
    write_varint(p, 10);
    write_varint(p, 10);

    // 11. Reduced Debug Info & Enable Respawn Screen
    write_byte(p, 0);
    write_byte(p, 1);

    // 12. Is Debug & Is Flat
    write_byte(p, 0);
    write_byte(p, 0);

    // 13. Last Death Location (Has Death Location = false)
    write_byte(p, 0);

    // 14. Portal Cooldown
    write_varint(p, 0);

    // 发送
    plog(LOG_INFO "Sending Login (Play) packet. Size: %zu bytes", p->size);

    netbuf_send(conn->socket, p);
    netbuf_free(p);
}

void send_player_position(client_connection *conn) {
    netbuf *p = netbuf_new(128);
    
    // Packet ID: 0x3C (Synchronize Player Position)
    write_varint(p, 0x3C);

    // X, Y, Z (Double)
    write_double_to_buf(p, 0.0);    // X
    write_double_to_buf(p, 100.0);  // Y (高一点防止掉进虚空)
    write_double_to_buf(p, 0.0);    // Z

    // Yaw, Pitch (Float)
    write_float_to_buf(p, 0.0f);
    write_float_to_buf(p, 0.0f);

    // Flags (Byte) - 0 表示所有坐标都是绝对坐标
    write_byte(p, 0);

    // Teleport ID (VarInt) - 随便给一个
    write_varint(p, 1);

    netbuf_send(conn->socket, p);
    netbuf_free(p);
}

void send_keep_alive(client_connection *conn) {
    netbuf *p = netbuf_new(16);
    
    // Packet ID: 0x23 (Keep Alive)
    write_varint(p, 0x23);
    
    // Keep Alive ID (Long) - 用当前时间戳
    write_long_to_buf(p, 12345678L);
    
    netbuf_send(conn->socket, p);
    netbuf_free(p);
    
    plog(LOG_INFO "Sent Keep Alive");
}

void send_player_info_update(client_connection *conn) {
    netbuf *p = netbuf_new(256);
    
    // Packet ID: 0x3A (Player Info Update)
    write_varint(p, 0x3A);
    
    // Action: Add Player (0x01) | Update Listed (0x08) | Update Latency (0x20)
    write_byte(p, 0x01 | 0x08 | 0x20);  // = 0x29
    
    // Number of players
    write_varint(p, 1);
    
    // Player UUID
    uuid_t uuid;
    memset(uuid, 0, 16);
    size_t name_len = strlen(conn->player_name);
    for (size_t i = 0; i < name_len && i < 16; i++) {
        uuid[i] = (unsigned char)conn->player_name[i];
    }
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x0f) | 0xa0;
    
    for (int i = 0; i < 16; i++) write_byte(p, uuid[i]);
    
    // Add Player action
    write_string(p, conn->player_name);
    write_varint(p, 0);  // Number of properties
    
    // Update Listed action
    write_byte(p, 1);  // Listed = true
    
    // Update Latency action
    write_varint(p, 0);  // Latency = 0 (0ms)
    
    netbuf_send(conn->socket, p);
    netbuf_free(p);
    
    plog(LOG_INFO "Sent Player Info Update");
}

void send_game_event(client_connection *conn, uint8_t event, float value) {
    netbuf *p = netbuf_new(16);
    
    // Packet ID: 0x1F (Game Event)
    write_varint(p, 0x1F);
    
    // Event type
    write_byte(p, event);
    
    // Value (Float)
    write_float_to_buf(p, value);
    
    netbuf_send(conn->socket, p);
    netbuf_free(p);
    
    plog(LOG_INFO "Sent Game Event: %d", event);
}

void send_chunk_data(client_connection *conn, int32_t chunk_x, int32_t chunk_z) {
    netbuf *p = netbuf_new(4096);
    
    // Packet ID: 0x24 (Chunk Data)
    write_varint(p, 0x24);
    
    // Chunk X, Z (Int)
    write_be32_to_buf(p, chunk_x);
    write_be32_to_buf(p, chunk_z);
    
    // Heightmaps NBT (empty compound with TAG_End)
    write_byte(p, 0x0A);  // TAG_Compound
    write_byte(p, 0x00);  // Empty name length
    write_byte(p, 0x00);
    write_byte(p, 0x00);  // TAG_End
    
    // Chunk data size (VarInt)
    // 32 sections * 6 bytes + 4 extra bytes = 196 bytes
    write_varint(p, 196);
    
    // Chunk data: 32 sections
    for (int i = 0; i < 32; i++) {
        // Block count (Short) - 0 blocks
        write_byte(p, 0x00);
        write_byte(p, 0x00);
        
        // Blocks: bits per entry = 0, single value = 0 (air)
        write_byte(p, 0x00);  // bits per entry
        write_byte(p, 0x00);  // single value
        
        // Biomes: bits per entry = 0, single value = 0 (plains)
        write_byte(p, 0x00);  // bits per entry
        write_byte(p, 0x00);  // single value
    }
    
    // 4 extra bytes (参考项目格式)
    write_byte(p, 0x00);
    write_byte(p, 0x00);
    write_byte(p, 0x00);
    write_varint(p, 0);
    
    // Block entities count (VarInt)
    write_varint(p, 0);
    
    // Light data - 按参考项目格式
    for (int i = 0; i < 4; i++) {
        write_varint(p, 1);
        write_long_to_buf(p, 0);
    }
    
    // Sky Light array count
    write_varint(p, 0);
    
    // Block Light array count
    write_varint(p, 0);
    
    netbuf_send(conn->socket, p);
    netbuf_free(p);
    
    plog(LOG_INFO "Sent Chunk Data: (%d, %d)", chunk_x, chunk_z);
}

void send_chunk_center(client_connection *conn, int32_t chunk_x, int32_t chunk_z) {
    netbuf *p = netbuf_new(16);
    
    // Packet ID: 0x4E (Chunk Center)
    write_varint(p, 0x4E);
    
    // Chunk X, Z (VarInt)
    write_varint(p, chunk_x);
    write_varint(p, chunk_z);
    
    netbuf_send(conn->socket, p);
    netbuf_free(p);
    
    plog(LOG_INFO "Sent Chunk Center: (%d, %d)", chunk_x, chunk_z);
}

void send_world_loading(client_connection *conn) {
    plog(LOG_INFO "Starting world loading sequence...");
    
    // 1. Keep Alive
    send_keep_alive(conn);
    
    // 2. Player Info Update (Tab List)
    send_player_info_update(conn);
    
    // 3. Send chunks around spawn (0, 0)
    for (int x = -1; x <= 1; x++) {
        for (int z = -1; z <= 1; z++) {
            send_chunk_data(conn, x, z);
        }
    }
    
    // 4. Chunk Center
    send_chunk_center(conn, 0, 0);
    
    // 5. Game Event: Start Waiting for Chunks (type 13) - 必须在chunks之后
    send_game_event(conn, 13, 0.0f);
    
    // 6. Player Position
    send_player_position(conn);
    
    plog(LOG_INFO "World loading sequence complete");
}
