#include <stddef.h>
#include <stdio.h>

#include "login.h"
#include "netbuf.h"
#include "nbt.h"
#include "protocol.h"
#include "varint.h"
#include "uuid.h"

// 构造一个最小化的 NBT 注册表，只包含主世界和平原生物群系
void write_minimal_registry(netbuf *buf) {
    nbt_begin_root_compound(buf); // 根节点 (匿名)

    // --- 1. dimension_type ---
    nbt_begin_compound(buf, "minecraft:dimension_type");
        nbt_write_string(buf, "type", "minecraft:dimension_type");
        nbt_begin_list(buf, "value", TAG_Compound, 1);
            nbt_begin_compound(buf, NULL);
                nbt_write_string(buf, "name", "minecraft:overworld");
                nbt_write_int(buf, "id", 0);
                nbt_begin_compound(buf, "element");
                    nbt_write_byte(buf, "piglin_safe", 0);
                    nbt_write_byte(buf, "has_ceiling", 0);
                    nbt_write_byte(buf, "has_skylight", 1);
                    nbt_write_byte(buf, "natural", 1);
                    nbt_write_float(buf, "ambient_light", 0.0f);
                    nbt_write_byte(buf, "has_raids", 1);
                    nbt_write_int(buf, "min_y", -64);
                    nbt_write_int(buf, "height", 384);
                    nbt_write_int(buf, "logical_height", 384);
                    nbt_write_string(buf, "infiniburn", "#minecraft:infiniburn_overworld");
                    nbt_write_string(buf, "effects", "minecraft:overworld");
                    nbt_write_int(buf, "monster_spawn_light_level", 0); // 更改为 Int 标签
                    nbt_write_int(buf, "monster_spawn_block_light_limit", 0);
                    nbt_write_double(buf, "coordinate_scale", 1.0);
                    nbt_write_byte(buf, "bed_works", 1);
                    nbt_write_byte(buf, "respawn_anchor_works", 0);
                nbt_end_compound(buf); // element end
            nbt_end_compound(buf); // list item end
        nbt_end_compound(buf); // value end
    nbt_end_compound(buf); // dimension_type end

    // --- 2. worldgen/biome ---
    nbt_begin_compound(buf, "minecraft:worldgen/biome");
        nbt_write_string(buf, "type", "minecraft:worldgen/biome");
        nbt_begin_list(buf, "value", TAG_Compound, 1);
            nbt_begin_compound(buf, NULL);
                nbt_write_string(buf, "name", "minecraft:plains");
                nbt_write_int(buf, "id", 0);
                nbt_begin_compound(buf, "element");
                    nbt_write_string(buf, "precipitation", "none"); // 简单起见写 none
                    nbt_write_float(buf, "temperature", 0.8f);
                    nbt_write_float(buf, "downfall", 0.4f);
                    nbt_begin_compound(buf, "effects");
                        nbt_write_int(buf, "sky_color", 7907327);
                        nbt_write_int(buf, "water_color", 4159231);
                        nbt_write_int(buf, "fog_color", 12638463);
                        nbt_write_int(buf, "water_fog_color", 329011);
                    nbt_end_compound(buf); // effects end
                nbt_end_compound(buf); // element end
            nbt_end_compound(buf); // list item end
        nbt_end_compound(buf); // value end
    nbt_end_compound(buf); // biome end

    // --- 3. chat_type (占位符) ---
    nbt_begin_compound(buf, "minecraft:chat_type");
        nbt_write_string(buf, "type", "minecraft:chat_type");
        nbt_begin_list(buf, "value", TAG_Compound, 0); // 空列表也可以
        nbt_end_compound(buf);
    nbt_end_compound(buf);

    // --- 4. damage_type (占位符) ---
    nbt_begin_compound(buf, "minecraft:damage_type");
        nbt_write_string(buf, "type", "minecraft:damage_type");
        nbt_begin_list(buf, "value", TAG_Compound, 0);
        nbt_end_compound(buf);
    nbt_end_compound(buf);

    nbt_end_compound(buf); // 整个 Registry 根结束
}

void send_login_success(client_connection *conn) {
    /* Send Login Success (ID 0x02) */
    netbuf *resp = netbuf_new(1024);
    write_varint(resp, 0x02); 

    // 写入 UUID (离线 UUID 格式通常是全 0 或者根据名字生成的固定值)
    // 这里我们先写一个固定测试用的 UUID (16字节二进制)
    uuid_t uuid;
    uuid_generate_random(uuid); 
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
    write_be32_to_buf(p, 100); // 随便给个 ID

    // 2. Is Hardcore
    write_byte(p, 0);

    // 3. Dimension Names (Array of Identifier)
    write_varint(p, 1); // 只有一个维度
    write_string(p, "minecraft:overworld");

    // 4. Registry Data (就是上面那一坨 NBT)
    write_minimal_registry(p);

    // 5. Dimension Type & Dimension Name
    write_string(p, "minecraft:overworld");
    write_string(p, "minecraft:overworld");

    // 6. Hashed Seed
    write_long_to_buf(p, 12345678L);

    // 7. Max Players (VarInt)
    write_varint(p, 20);

    // 8. View Distance & Simulation Distance (VarInt)
    write_varint(p, 10);
    write_varint(p, 10);

    // 9. Reduced Debug Info & Enable Respawn Screen
    write_byte(p, 0);
    write_byte(p, 1);

    // 10. Is Debug & Is Flat
    write_byte(p, 0);
    write_byte(p, 0);

    // 11. Last Death Location (Has Death Location = false)
    write_byte(p, 0);

    // 12. Portal Cooldown
    write_varint(p, 0);

    // 发送
    printf("Sending Login (Play) packet. Size: %zu bytes\n", p->size);

    for (int i = 0; i < 64 && i < p->size; i++) printf("%02X ", p->data[i]);
    printf("\n");

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
