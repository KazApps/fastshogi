#pragma once

namespace fastshogi {

enum class NotationType { SAN, LAN, UCI };
enum class OrderType { RANDOM, SEQUENTIAL };
enum class FormatType { EPD, PGN, NONE };
enum class VariantType { STANDARD, FRC };
enum class TournamentType { ROUNDROBIN, GAUNTLET };
enum class OutputType { FASTSHOGI, CUTECHESS, NONE };

}  // namespace fastshogi