#pragma once
#include <SFML/Graphics/Image.hpp>
#include <filesystem>
namespace MapShotImageWriter {
int JpegQuality(int compression);
bool Save(const sf::Image& image, const std::filesystem::path& path, int option);
}
