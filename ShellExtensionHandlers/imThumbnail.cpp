#include <SFML/Graphics.hpp>
#include <windows.h>

HBITMAP CreateThumbnail(const char* filename, UINT thumb_size, BOOL &has_alpha)
{
  int error;
  sf::RenderTexture thumbnail;
  thumbnail.create(thumb_size, thumb_size);
  thumbnail.clear(sf::Color::Transparent);
  sf::CircleShape circle(thumb_size / 2);
  circle.setFillColor(sf::Color::Red);
  //circle.setPosition(thumb_size / 2, thumb_size / 2);
  thumbnail.draw(circle);
  thumbnail.display();

  sf::Image img = thumbnail.getTexture().copyToImage();
  const sf::Uint8* pixels = img.getPixelsPtr();

  // Create the bitmap that will hold our color data
  BITMAPV5HEADER bitmapHeader;
  std::memset(&bitmapHeader, 0, sizeof(BITMAPV5HEADER));

  bitmapHeader.bV5Size = sizeof(BITMAPV5HEADER);
  bitmapHeader.bV5Width = thumb_size;
  bitmapHeader.bV5Height = -static_cast<int>(thumb_size); // Negative indicates origin is in upper-left corner
  bitmapHeader.bV5Planes = 1;
  bitmapHeader.bV5BitCount = 32;
  bitmapHeader.bV5Compression = BI_BITFIELDS;
  bitmapHeader.bV5RedMask = 0x00ff0000;
  bitmapHeader.bV5GreenMask = 0x0000ff00;
  bitmapHeader.bV5BlueMask = 0x000000ff;
  bitmapHeader.bV5AlphaMask = 0xff000000;

  sf::Uint32* bitmapData = NULL;

  HDC screenDC = GetDC(NULL);
  HBITMAP color = CreateDIBSection(
	  screenDC,
	  reinterpret_cast<const BITMAPINFO*>(&bitmapHeader),
	  DIB_RGB_COLORS,
	  reinterpret_cast<void**>(&bitmapData),
	  NULL,
	  0
  );
  ReleaseDC(NULL, screenDC);

  // Fill our bitmap with the cursor color data
  // We'll have to swap the red and blue channels here
  sf::Uint32* bitmapOffset = bitmapData;
  for (std::size_t remaining = thumb_size * thumb_size; remaining > 0; --remaining, pixels += 4)
  {
	  *bitmapOffset++ = (pixels[3] << 24) | (pixels[0] << 16) | (pixels[1] << 8) | pixels[2];
  }

  has_alpha = TRUE;
  
  return color;
}
