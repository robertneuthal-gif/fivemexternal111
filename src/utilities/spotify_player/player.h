#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <atomic>
#include <thread>

class Spotify {
public:
  Spotify();
  ~Spotify();

  bool update();

  const std::wstring& author() const;
  const std::wstring& title() const;
  bool isPlaying() const;

  static void startSpotifyUpdateThread();

  // Static members
  static std::wstring g_cachedAuthor;  // This will be shared across all instances
  static std::wstring g_cachedTitle;
  static std::atomic<bool> g_isPlaying;

private:
  void updateWindow();
  void updateTitle(bool& changed);
  void updateString(std::wstring& str, const std::wstring& newStr, bool& changed);

  std::wstring mAuthor;
  std::wstring mTitle;

  static void spotifyUpdateThread();
};

void renderSpotifyWatermark();

#endif // PLAYER_H
