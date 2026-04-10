#include <iostream>
#include <string>

// ========== 子系统组件（各自独立，各自复杂） ==========

class Amplifier {
public:
    void on()  { std::cout << "  Amplifier: ON\n"; }
    void off() { std::cout << "  Amplifier: OFF\n"; }
    void setVolume(int level) {
        std::cout << "  Amplifier: Volume set to " << level << "\n";
    }
    void setSurroundSound() {
        std::cout << "  Amplifier: Surround sound enabled\n";
    }
};

class BluRayPlayer {
public:
    void on()  { std::cout << "  BluRay: ON\n"; }
    void off() { std::cout << "  BluRay: OFF\n"; }
    void play(const std::string& movie) {
        std::cout << "  BluRay: Playing '" << movie << "'\n";
    }
    void stop() { std::cout << "  BluRay: Stopped\n"; }
    void eject() { std::cout << "  BluRay: Disc ejected\n"; }
};

class Projector {
public:
    void on()  { std::cout << "  Projector: ON\n"; }
    void off() { std::cout << "  Projector: OFF\n"; }
    void setWideScreen() {
        std::cout << "  Projector: Widescreen mode (16:9)\n";
    }
    void setInput(const std::string& source) {
        std::cout << "  Projector: Input set to " << source << "\n";
    }
};

class Screen {
public:
    void down() { std::cout << "  Screen: Lowered\n"; }
    void up()   { std::cout << "  Screen: Raised\n"; }
};

class Lights {
public:
    void dim(int level) {
        std::cout << "  Lights: Dimmed to " << level << "%\n";
    }
    void on() { std::cout << "  Lights: ON (100%)\n"; }
};

// ========== 外观类 ==========
class HomeTheaterFacade {
private:
    Amplifier    amp_;
    BluRayPlayer player_;
    Projector    projector_;
    Screen       screen_;
    Lights       lights_;

public:
    // 高层接口：看电影
    void watchMovie(const std::string& movie) {
        std::cout << "=== Preparing to watch: " << movie << " ===\n";
        lights_.dim(10);
        screen_.down();
        projector_.on();
        projector_.setWideScreen();
        projector_.setInput("BluRay");
        amp_.on();
        amp_.setSurroundSound();
        amp_.setVolume(7);
        player_.on();
        player_.play(movie);
        std::cout << "=== Enjoy your movie! ===\n";
    }

    // 高层接口：结束观影
    void endMovie() {
        std::cout << "\n=== Shutting down movie theater ===\n";
        player_.stop();
        player_.eject();
        player_.off();
        amp_.off();
        projector_.off();
        screen_.up();
        lights_.on();
        std::cout << "=== Goodnight! ===\n";
    }

    // 高层接口：听音乐（复用部分子系统）
    void listenToMusic() {
        std::cout << "=== Music mode ===\n";
        lights_.dim(50);
        amp_.on();
        amp_.setVolume(5);
        std::cout << "=== Ready for music! ===\n";
    }
};

// ========== 客户端代码 ==========

int main() {
    HomeTheaterFacade theater;

    // 客户端只需调用一个方法——不关心 5 个子系统的操作顺序
    theater.watchMovie("The Matrix");
    // Output:
    // === Preparing to watch: The Matrix ===
    //   Lights: Dimmed to 10%
    //   Screen: Lowered
    //   Projector: ON
    //   Projector: Widescreen mode (16:9)
    //   Projector: Input set to BluRay
    //   Amplifier: ON
    //   Amplifier: Surround sound enabled
    //   Amplifier: Volume set to 7
    //   BluRay: ON
    //   BluRay: Playing 'The Matrix'
    // === Enjoy your movie! ===

    theater.endMovie();
    // === Shutting down movie theater ===
    //   BluRay: Stopped
    //   BluRay: Disc ejected
    //   BluRay: OFF
    //   Amplifier: OFF
    //   Projector: OFF
    //   Screen: Raised
    //   Lights: ON (100%)
    // === Goodnight! ===
}