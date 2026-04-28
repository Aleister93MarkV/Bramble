QT += core gui widgets network

TARGET = Bramble
CONFIG += c++20

INCLUDEPATH += include

LIBS += -lsndfile -lopenmpt

SOURCES += \
    src_cpp/main.cpp \
    src_cpp/MainWindow.cpp \
    src_cpp/AudioEngine.cpp \
    src_cpp/EqualizerWindow.cpp \
    src_cpp/VisualizerWindow.cpp \
    src_cpp/SkinWindow.cpp \
    src_cpp/foobar2k_sdk/dsp_manager.cpp \
    src_cpp/foobar2k_sdk/eq_dsp.cpp \
    src_cpp/foobar2k_sdk/crystallizer_dsp.cpp

HEADERS += \
    src_cpp/MainWindow.hpp \
    src_cpp/AudioEngine.hpp \
    src_cpp/EqualizerWindow.hpp \
    src_cpp/VisualizerWindow.hpp \
    src_cpp/SkinWindow.hpp \
    src_cpp/SkinManager.hpp \
    src_cpp/foobar2k_sdk/service_base.hpp \
    src_cpp/foobar2k_sdk/dsp.hpp \
    src_cpp/foobar2k_sdk/dsp_manager.hpp \
    src_cpp/foobar2k_sdk/eq_dsp.hpp \
    src_cpp/foobar2k_sdk/crystallizer_dsp.hpp

# Optimization Flags for Performance
QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -ffast-math
