TARGET := projeto-so.exe
BUILD := build

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# modifique pro diretório de instalação do Qt no seu sistema
QT_DIR   := C:/Qt/6.11.1/mingw_64

QT_INC   := -I$(QT_DIR)/include \
             -I$(QT_DIR)/include/QtCore \
             -I$(QT_DIR)/include/QtWidgets \
             -I$(QT_DIR)/include/QtGui

QT_LIBS  := -L$(QT_DIR)/lib \
             -lQt6Widgets \
             -lQt6Gui \
             -lQt6Core

MOC      := $(QT_DIR)/bin/moc.exe
UIC      := $(QT_DIR)/bin/uic.exe

# Força o uso do cmd.exe como shell, independente do que estiver no PATH
# (evita conflito com sh.exe de outras instalações, ex: w64devkit/raylib)
SHELL      := cmd.exe
.SHELLFLAGS := /C

SRCS := main.cpp \
        mainwindow.cpp \
        escalonador.cpp \
        relatoriowindow.cpp \
        Escalonador/auxiliares.cpp \
        Escalonador/base.cpp \
        Escalonador/priori.cpp \
        Escalonador/rr.cpp \
        Escalonador/sjf.cpp \
        Memoria/GerenciadorMemoria.cpp

UI_FILES := mainwindow.ui escalonador.ui relatoriowindow.ui
UI_HEADERS := $(patsubst %.ui,$(BUILD)/ui_%.h,$(notdir $(UI_FILES)))

MOC_HEADERS := mainwindow.h escalonador.h relatoriowindow.h
MOC_SRCS := $(patsubst %.h,$(BUILD)/moc_%.cpp,$(notdir $(MOC_HEADERS)))

OBJS := $(patsubst %.cpp,$(BUILD)/%.o,$(SRCS))
MOC_OBJS := $(patsubst $(BUILD)/%.cpp,$(BUILD)/%.o,$(MOC_SRCS))
ALL_OBJS := $(OBJS) $(MOC_OBJS)

ALL_CXXFLAGS := $(CXXFLAGS) $(QT_INC) -I. -IEscalonador -IMemoria -I$(BUILD)

.PHONY: all run clean mrproper

all: $(TARGET)

run: all
	set "PATH=$(subst /,\,$(QT_DIR))\bin;%PATH%" && $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CXX) $(ALL_CXXFLAGS) -o $@ $^ $(QT_LIBS)

$(BUILD)/%.o: %.cpp $(UI_HEADERS)
	if not exist "$(BUILD)" mkdir "$(BUILD)"
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

$(BUILD)/Escalonador/%.o: Escalonador/%.cpp $(UI_HEADERS)
	if not exist "$(BUILD)\Escalonador" mkdir "$(BUILD)\Escalonador"
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

$(BUILD)/Memoria/%.o: Memoria/%.cpp $(UI_HEADERS)
	if not exist "$(BUILD)\Memoria" mkdir "$(BUILD)\Memoria"
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

$(BUILD)/moc_%.o: $(BUILD)/moc_%.cpp $(UI_HEADERS)
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

$(BUILD)/ui_mainwindow.h: mainwindow.ui
	if not exist "$(BUILD)" mkdir "$(BUILD)"
	$(UIC) $< -o $@

$(BUILD)/ui_escalonador.h: escalonador.ui
	if not exist "$(BUILD)" mkdir "$(BUILD)"
	$(UIC) $< -o $@

$(BUILD)/ui_relatoriowindow.h: relatoriowindow.ui
	if not exist "$(BUILD)" mkdir "$(BUILD)"
	$(UIC) $< -o $@

$(BUILD)/moc_mainwindow.cpp: mainwindow.h $(UI_HEADERS)
	$(MOC) $(QT_INC) -I$(BUILD) $< -o $@

$(BUILD)/moc_escalonador.cpp: escalonador.h $(UI_HEADERS)
	$(MOC) $(QT_INC) -I$(BUILD) $< -o $@

$(BUILD)/moc_relatoriowindow.cpp: relatoriowindow.h $(UI_HEADERS)
	$(MOC) $(QT_INC) -I$(BUILD) $< -o $@

clean:
	if exist "$(BUILD)" rd /s /q "$(BUILD)"

mrproper: clean
	if exist "$(TARGET)" del /f "$(TARGET)"