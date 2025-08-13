#pragma once

namespace fastshogi {

enum class NotationType { SAN, LAN, USI };
enum class OrderType { RANDOM, SEQUENTIAL };
enum class FormatType { EPD, PGN, NONE };
enum class TournamentType { ROUNDROBIN, GAUNTLET };

}  // namespace fastshogi