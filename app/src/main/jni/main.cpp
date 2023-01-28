#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Main.hpp>
#include <iostream>
#include <android/log.h>
#include <string>

// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
// Uncomment this to try JNI access; this seems to be broken in latest NDKs
//#define USE_JNI

#if defined(USE_JNI)
// These headers are only needed for direct NDK/JDK interaction
#include <android/native_activity.h>
#include <jni.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

// NDK/JNI sub example - call Java code from native code
int vibrate(sf::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity* activity = sf::getNativeActivity();

    // Retrieve the JVM and JNI environment
    JavaVM* vm  = activity->vm;
    JNIEnv* env = activity->env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name    = "NativeThread";
    attachargs.group   = nullptr;
    jint res           = vm->AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    // Retrieve class information
    jclass natact  = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");

    // Get the value of a constant
    jfieldID fid    = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject  svcstr = env->GetStaticObjectField(context, fid);

    // Get the method 'getSystemService' and call it
    jmethodID getss   = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject   vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    // Get the object's class and retrieve the member name
    jclass    vib_cls = env->GetObjectClass(vib_obj);
    jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V");

    // Determine the timeframe
    jlong length = duration.asMilliseconds();

    // Bzzz!
    env->CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env->DeleteLocalRef(vib_obj);
    env->DeleteLocalRef(vib_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    // Detach thread again
    vm->DetachCurrentThread();
}
#endif

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)
using namespace sf;
using namespace std;

int ground = 628;

const int h = 24;
const int w = 46;

//String tileMap[12] = {
//        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
//        "B                                       B    B",
//        "B                                       B    B",
//        "B                                       B    B",
//        "B                                       B    B",
//        "B          0000                      BBBB    B",
//        "B                                       B    B",
//        "BBB                                     B    B",
//        "B                BB                     BB   B",
//        "B                BB                          B",
//        "B                BB             BB           B",
//        "BBBBBBBBB     BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
//};

string tileMap[24] = {
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        "B                                       B    B",
        "B                                       B    B",
        "B                                       B    B",
        "B                                       B    B",
        "B          0000                      BBBB    B",
        "B                                       B    B",
        "BBB                                     B    B",
        "B                BB                     BB   B",
        "B                BB                          B",
        "B                BB             BB           B",
        "B             BBBBBB          BBBBBBBBBBBBBBBB",
        "B                                            B",
        "B                                       B    B",
        "B                                       B    B",
        "B                                       B    B",
        "B                                       B    B",
        "B          0000                      BBBB    B",
        "B                                       B    B",
        "BBB                                     B    B",
        "B                BB                     BB   B",
        "B                BB                          B",
        "B                BB             BB           B",
        "BBBBBBBBB     BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
};

class Player {

public:
    float dx, dy;
    FloatRect rect;
    bool onGround;
    Sprite sprite;
    float currentFrame;

    Player(Texture &texture) {
        sprite.setTexture(texture);
        rect = FloatRect({0, 628}, {108, 140});
        sprite.setTextureRect(
                IntRect(
                        {0, 0},
                        {(int) rect.width, (int) rect.height}
                )
        );
        dx = dy = 0;
        currentFrame = 0;
        sprite.setPosition(rect.getPosition());
        onGround = true;
    }

    void update(float time) {
        rect.left += (dx * time);

        if (!onGround) {
            dy += 0.002 * time;
            rect.top += dy * time;
            if (rect.top > ground) {
                rect.top = ground;
                dy = 0;
                onGround = true;
            }
        }
        //animation
        currentFrame += 0.01 * time;
        if (currentFrame > 8) currentFrame -= 8;
        if (dx > 0) {
            sprite.setTextureRect(IntRect({108 * (int) currentFrame, 0}, {108, 140}));
        } else if (dx < 0) {
            sprite.setTextureRect(IntRect({108 * (int) currentFrame, 140}, {108, 140}));
        }
        //positioning
        sprite.setPosition(rect.getPosition());
        dx = 0;
    }
};

void HandleMoveTap(Player &player, int primaryFinger) {
    if (Touch::isDown(primaryFinger)) {
        int x = Touch::getPosition(primaryFinger).x;
        if (x > 200 && x < 400) {
            player.dx = 0.5f;
        } else if (x < 200) {
            player.dx = -0.5f;
        }
    }
}

void HandleJumpTap(Player &player, int primaryFinger, int secondaryFinger) {
    if ((Touch::isDown(primaryFinger) && Touch::getPosition(primaryFinger).x > 600) ||
        (Touch::isDown(secondaryFinger) && Touch::getPosition(secondaryFinger).x > 600)) {
        if (player.onGround) {
            player.onGround = false;
            player.dy = -0.9f;
        }
    }
}

int main(int argc, char *argv[]) {
    RenderWindow window(VideoMode({1184, 768}), "Test!", Style::Fullscreen);
    Texture texture;
    texture.loadFromFile("scottpilgrim_multiple.png");
    Player player(texture);
    Clock clock;
    RectangleShape rectangleShape;
    rectangleShape.setSize({32, 32});
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asMilliseconds();
        clock.restart();
        time = time * 1;
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }
        HandleMoveTap(player, 0);
        HandleJumpTap(player, 0, 1);
        player.update(time);
        window.clear(Color(255, 255, 255));
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                char cell = tileMap[i][j];
                if (cell == 'B') rectangleShape.setFillColor(Color::Black);
                else if (cell == '0') rectangleShape.setFillColor(Color::Green);
                else if (cell == ' ') continue;
                rectangleShape.setPosition({j * 32.0f, i * 32.0f});
                window.draw(rectangleShape);
            }
        }
        window.draw(rectangleShape);
        window.draw(player.sprite);
        window.display();
    }
    return 0;
}
