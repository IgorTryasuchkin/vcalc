// Модульные тесты проекта vcalc (C++17, стандартная библиотека)
// Сборка и запуск: make test
// Тестируются модули: Calculator, sha256, Authenticator, ClientBase,
// DataSerializer, Logger, UserInterface/ServerConfig.
// Сетевые модули (NetworkServer, SessionHandler) и main.cpp модульными тестами
// не покрываются: им требуются сокеты и клиент.
#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <string>
#include <vector>

#include "Authenticator.h"
#include "Calculator.h"
#include "ClientBase.h"
#include "DataSerializer.h"
#include "Logger.h"
#include "ServerConfig.h"
#include "UserInterface.h"
#include "sha256.h"

static int g_total = 0, g_passed = 0, g_failed = 0;

// Выполнить один тест; любое исключение, вышедшее из тела теста, считается провалом.
static void run(int id, const char* title, const std::function<bool()>& body) {
    ++g_total;
    std::cout << "T" << id << " RUN   " << title << std::endl;
    bool ok = false;
    try { ok = body(); } catch (const std::exception& e) {
        std::cout << "      исключение: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "      неизвестное исключение" << std::endl;
    }
    if (ok) ++g_passed; else ++g_failed;
    std::cout << "T" << id << (ok ? " PASS" : " FAIL") << std::endl;
}

static std::string tmpPath(const std::string& name) { return "/tmp/vcalc_ut_" + name; }

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream o(path, std::ios::binary);
    o << content;
}

static std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static bool contains(const std::string& s, const std::string& sub) {
    return s.find(sub) != std::string::npos;
}

// Вызов UserInterface::parseArgs с подавлением вывода справки/ошибок.
static bool parseArgs(const std::vector<std::string>& args, ServerConfig* out) {
    std::vector<const char*> argv;
    argv.push_back("vcalc");
    for (const auto& a : args) argv.push_back(a.c_str());
    std::ostringstream sink;
    auto* oldOut = std::cout.rdbuf(sink.rdbuf());
    auto* oldErr = std::cerr.rdbuf(sink.rdbuf());
    UserInterface ui;
    bool r = ui.parseArgs((int)argv.size(), argv.data());
    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);
    if (out) *out = ui.getConfig();
    return r;
}

int main() {
    auto t0 = std::chrono::steady_clock::now();
    std::cout << "=== МОДУЛЬНОЕ ТЕСТИРОВАНИЕ vcalc ===\n" << std::endl;
    std::cout << "\n[Класс Calculator]" << std::endl;
    run(1, "Пустой вектор", []() -> bool {
        Calculator c;
        return c.sumVector({}) == 0;
    });
    run(2, "Вектор из одного элемента", []() -> bool {
        Calculator c;
        return c.sumVector({999999}) == 999999;
    });
    run(3, "Базовая сумма", []() -> bool {
        Calculator c;
        return c.sumVector({100, 200, 300, 400}) == 1000;
    });
    run(4, "Отрицательные значения", []() -> bool {
        Calculator c;
        return c.sumVector({-5, -10, -15}) == -30;
    });
    run(5, "Взаимное уничтожение знаков", []() -> bool {
        Calculator c;
        return c.sumVector({5, -5}) == 0;
    });
    run(6, "Насыщение при переполнении вверх", []() -> bool {
        Calculator c;
        return c.sumVector({INT32_MAX, 10}) == INT32_MAX;
    });
    run(7, "Насыщение при переполнении вниз", []() -> bool {
        Calculator c;
        return c.sumVector({INT32_MIN, -10}) == INT32_MIN;
    });
    run(8, "Точное достижение максимума", []() -> bool {
        Calculator c;
        return c.sumVector({2147483646, 1}) == INT32_MAX;
    });
    run(9, "Точное достижение минимума", []() -> bool {
        Calculator c;
        return c.sumVector({-2147483647, -1}) == INT32_MIN;
    });
    run(10, "Промежуточная сумма вне int32 (64-битный аккумулятор)", []() -> bool {
        Calculator c;
        return c.sumVector({INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN}) == -2;
    });
    run(11, "checkOverflow: значение больше максимума", []() -> bool {
        Calculator c;
        return c.checkOverflow((int64_t)INT32_MAX + 1) == INT32_MAX;
    });
    run(12, "checkOverflow: значение меньше минимума", []() -> bool {
        Calculator c;
        return c.checkOverflow((int64_t)INT32_MIN - 1) == INT32_MIN;
    });
    run(13, "checkOverflow: значение в допустимом диапазоне", []() -> bool {
        Calculator c;
        return c.checkOverflow(123) == 123 && c.checkOverflow(-123) == -123;
    });
    std::cout << "\n[Модуль sha256]" << std::endl;
    run(14, "Хеш пустой строки", []() -> bool {
        return sha256Hex("") == "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
    });
    run(15, "Эталонный вектор NIST «abc»", []() -> bool {
        return sha256Hex("abc") == "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD";
    });
    run(16, "Эталонный вектор NIST из двух блоков", []() -> bool {
        return sha256Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") == "248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1";
    });
    run(17, "Граница дополнения: 55 байт", []() -> bool {
        return sha256Hex(std::string(55, 'a')) == "9F4390F8D30C2DD92EC9F095B65E2B9AE9B0A925A5258E241C9F1E910F734318";
    });
    run(18, "Граница дополнения: 56 байт", []() -> bool {
        return sha256Hex(std::string(56, 'a')) == "B35439A4AC6F0948B6D6F9E3C6AF0F5F590CE20F1BDE7090EF7970686EC6738A";
    });
    run(19, "Граница дополнения: 64 байта (ровно один блок)", []() -> bool {
        return sha256Hex(std::string(64, 'a')) == "FFE054FE7AE0CB6DC65C3AF9B61D5209F439851DB43D0BA5997337DF154668EB";
    });
    run(20, "Формат результата: 64 символа, верхний регистр HEX", []() -> bool {
        std::string h = sha256Hex("test");
        if (h.size() != 64) return false;
        for (char ch : h) if (!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))) return false;
        return true;
    });
    std::cout << "\n[Класс Authenticator]" << std::endl;
    run(21, "formatSalt16: нулевая соль", []() -> bool {
        Authenticator a;
        return a.formatSalt16(0) == "0000000000000000";
    });
    run(22, "formatSalt16: дополнение нулями слева", []() -> bool {
        Authenticator a;
        return a.formatSalt16(0xABCDEFULL) == "0000000000ABCDEF";
    });
    run(23, "formatSalt16: максимальное значение", []() -> bool {
        Authenticator a;
        return a.formatSalt16(UINT64_MAX) == "FFFFFFFFFFFFFFFF";
    });
    run(24, "formatSalt16: верхний регистр и длина 16", []() -> bool {
        Authenticator a;
        std::string s = a.formatSalt16(0xabcdef0123456789ULL);
        return s == "ABCDEF0123456789" && s.size() == 16;
    });
    run(25, "generateSalt64: две соли различаются", []() -> bool {
        Authenticator a;
        uint64_t s1 = a.generateSalt64();
        uint64_t s2 = a.generateSalt64();
        return s1 != s2;
    });
    run(26, "hashPassword: длина хеша SHA-256", []() -> bool {
        Authenticator a;
        return a.hashPassword("A1B2C3D4E5F60789", "TestPass123").size() == 64;
    });
    run(27, "hashPassword: эталонное значение SHA-256(salt + password)", []() -> bool {
        Authenticator a;
        return a.hashPassword("A1B2C3D4E5F60789", "TestPass123") == "C7821BB369C931FB97EE4B47173160AF4B8BEB6956139705C6059D07E67F096E";
    });
    run(28, "hashPassword: разные соли дают разные хеши", []() -> bool {
        Authenticator a;
        return a.hashPassword("1111111111111111", "pw") != a.hashPassword("2222222222222222", "pw");
    });
    run(29, "verifyHash: верный пароль", []() -> bool {
        Authenticator a;
        std::string h = a.hashPassword("A1B2C3D4E5F60789", "TestPass123");
        return a.verifyHash("TestPass123", "A1B2C3D4E5F60789", h);
    });
    run(30, "verifyHash: неверный пароль", []() -> bool {
        Authenticator a;
        std::string h = a.hashPassword("A1B2C3D4E5F60789", "TestPass123");
        return !a.verifyHash("Wrong", "A1B2C3D4E5F60789", h);
    });
    run(31, "verifyHash: хеш клиента в нижнем регистре", []() -> bool {
        Authenticator a;
        std::string h = a.hashPassword("A1B2C3D4E5F60789", "TestPass123");
        for (char& ch : h) ch = (char)std::tolower((unsigned char)ch);
        return a.verifyHash("TestPass123", "A1B2C3D4E5F60789", h);
    });
    run(32, "verifyHash: другая соль", []() -> bool {
        Authenticator a;
        std::string h = a.hashPassword("1111111111111111", "pw");
        return !a.verifyHash("pw", "2222222222222222", h);
    });
    run(33, "Пустой пароль", []() -> bool {
        Authenticator a;
        std::string h = a.hashPassword("A1B2C3D4E5F60789", "");
        return h.size() == 64 && a.verifyHash("", "A1B2C3D4E5F60789", h);
    });
    run(34, "verifyHash: пустой хеш клиента", []() -> bool {
        Authenticator a;
        return !a.verifyHash("TestPass123", "A1B2C3D4E5F60789", "");
    });
    std::cout << "\n[Класс ClientBase]" << std::endl;
    run(35, "Загрузка одного пользователя", []() -> bool {
        std::string p = tmpPath("b1.conf");
        writeFile(p, "user1:pass1\n");
        ClientBase b;
        b.load(p);
        return b.checkLogin("user1") && b.getPasswordByLogin("user1") == "pass1" && b.size() == 1;
    });
    run(36, "Загрузка нескольких пользователей", []() -> bool {
        std::string p = tmpPath("b2.conf");
        writeFile(p, "a:1\nb:2\nc:3\n");
        ClientBase b;
        b.load(p);
        return b.size() == 3 && b.checkLogin("a") && b.checkLogin("b") && b.checkLogin("c") && !b.checkLogin("d");
    });
    run(37, "Пустой файл базы", []() -> bool {
        std::string p = tmpPath("b3.conf");
        writeFile(p, "");
        ClientBase b;
        b.load(p);
        return b.size() == 0 && !b.checkLogin("any");
    });
    run(38, "Несуществующий файл базы", []() -> bool {
        ClientBase b;
        try { b.load("/nonexistent_dir/no.conf"); } catch (const std::runtime_error&) { return true; }
        return false;
    });
    run(39, "Спецсимволы в пароле", []() -> bool {
        std::string p = tmpPath("b4.conf");
        writeFile(p, "user:P@ssW0rd!#$\n");
        ClientBase b;
        b.load(p);
        return b.getPasswordByLogin("user") == "P@ssW0rd!#$";
    });
    run(40, "Двоеточие внутри пароля", []() -> bool {
        std::string p = tmpPath("b5.conf");
        writeFile(p, "u:pa:ss\n");
        ClientBase b;
        b.load(p);
        return b.checkLogin("u") && b.getPasswordByLogin("u") == "pa:ss";
    });
    run(41, "Окончания строк CRLF", []() -> bool {
        std::string p = tmpPath("b6.conf");
        writeFile(p, "u1:p1\r\nu2:p2\r\n");
        ClientBase b;
        b.load(p);
        return b.getPasswordByLogin("u1") == "p1" && b.getPasswordByLogin("u2") == "p2";
    });
    run(42, "Пустые строки пропускаются", []() -> bool {
        std::string p = tmpPath("b7.conf");
        writeFile(p, "u1:p1\n\n\nu2:p2\n");
        ClientBase b;
        b.load(p);
        return b.size() == 2;
    });
    run(43, "Строка без разделителя", []() -> bool {
        std::string p = tmpPath("b8.conf");
        writeFile(p, "justtext\n");
        ClientBase b;
        try { b.load(p); } catch (const std::runtime_error&) { return true; }
        return false;
    });
    run(44, "Пустой логин", []() -> bool {
        std::string p = tmpPath("b9.conf");
        writeFile(p, ":password\n");
        ClientBase b;
        try { b.load(p); } catch (const std::runtime_error&) { return true; }
        return false;
    });
    run(45, "Пустой пароль допустим", []() -> bool {
        std::string p = tmpPath("b10.conf");
        writeFile(p, "user:\n");
        ClientBase b;
        b.load(p);
        return b.checkLogin("user") && b.getPasswordByLogin("user").empty();
    });
    run(46, "Пароль неизвестного логина", []() -> bool {
        std::string p = tmpPath("b11.conf");
        writeFile(p, "a:1\n");
        ClientBase b;
        b.load(p);
        return b.getPasswordByLogin("ghost").empty();
    });
    run(47, "Повторная загрузка очищает прежние данные", []() -> bool {
        std::string p1 = tmpPath("b12a.conf"), p2 = tmpPath("b12b.conf");
        writeFile(p1, "old:1\n");
        writeFile(p2, "new:2\n");
        ClientBase b;
        b.load(p1);
        b.load(p2);
        return !b.checkLogin("old") && b.checkLogin("new") && b.size() == 1;
    });
    run(48, "Дублирующийся логин", []() -> bool {
        std::string p = tmpPath("b13.conf");
        writeFile(p, "u:first\nu:second\n");
        ClientBase b;
        b.load(p);
        return b.size() == 1 && b.getPasswordByLogin("u") == "second";
    });
    run(49, "Логин чувствителен к регистру", []() -> bool {
        std::string p = tmpPath("b14.conf");
        writeFile(p, "User:1\n");
        ClientBase b;
        b.load(p);
        return b.checkLogin("User") && !b.checkLogin("user");
    });
    std::cout << "\n[Класс DataSerializer]" << std::endl;
    run(50, "serializeU32: размер и обратное преобразование", []() -> bool {
        auto b = DataSerializer::serializeU32(0x12345678u);
        return b.size() == 4 && DataSerializer::deserializeU32(b) == 0x12345678u;
    });
    run(51, "serializeU32: граничные значения", []() -> bool {
        return DataSerializer::deserializeU32(DataSerializer::serializeU32(0)) == 0u && DataSerializer::deserializeU32(DataSerializer::serializeU32(UINT32_MAX)) == UINT32_MAX;
    });
    run(52, "deserializeU32: слишком короткий вектор", []() -> bool {
        return DataSerializer::deserializeU32(std::vector<uint8_t>{1, 2, 3}) == 0u;
    });
    run(53, "deserializeU32: пустой вектор", []() -> bool {
        return DataSerializer::deserializeU32(std::vector<uint8_t>{}) == 0u;
    });
    run(54, "deserializeU32: вектор длиннее 4 байт", []() -> bool {
        auto b = DataSerializer::serializeU32(7);
        b.push_back(9);
        b.push_back(9);
        b.push_back(9);
        b.push_back(9);
        return DataSerializer::deserializeU32(b) == 7u;
    });
    run(55, "deserializeVector: 12 байт", []() -> bool {
        std::vector<int32_t> src{1, -2, 3};
        std::vector<uint8_t> raw(12);
        std::memcpy(raw.data(), src.data(), 12);
        return DataSerializer::deserializeVector(raw) == src;
    });
    run(56, "deserializeVector: пустой вектор", []() -> bool {
        return DataSerializer::deserializeVector(std::vector<uint8_t>{}).empty();
    });
    run(57, "deserializeVector: размер не кратен 4", []() -> bool {
        return DataSerializer::deserializeVector(std::vector<uint8_t>(7, 0)).size() == 1;
    });
    run(58, "serializeInt32: размер и круговое преобразование", []() -> bool {
        int32_t vals[] = {INT32_MIN, -1, 0, INT32_MAX};
        for (int32_t v : vals) { auto b = DataSerializer::serializeInt32(v); if (b.size() != 4) return false; auto r = DataSerializer::deserializeVector(b); if (r.size() != 1 || r[0] != v) return false; }
        return true;
    });
    run(59, "serializeResults: непустой вектор", []() -> bool {
        auto b = DataSerializer::serializeResults({1, 2, 3});
        if (b.size() != 16) return false;
        if (DataSerializer::deserializeU32(b) != 3u) return false;
        std::vector<uint8_t> tail(b.begin() + 4, b.end());
        return DataSerializer::deserializeVector(tail) == std::vector<int32_t>({1, 2, 3});
    });
    run(60, "serializeResults: пустой вектор", []() -> bool {
        auto b = DataSerializer::serializeResults({});
        return b.size() == 4 && DataSerializer::deserializeU32(b) == 0u;
    });
    run(61, "serializeResults: отрицательные значения", []() -> bool {
        auto b = DataSerializer::serializeResults({-1, INT32_MIN});
        std::vector<uint8_t> tail(b.begin() + 4, b.end());
        return DataSerializer::deserializeVector(tail) == std::vector<int32_t>({-1, INT32_MIN});
    });
    std::cout << "\n[Класс Logger]" << std::endl;
    run(62, "Запись критического сообщения", []() -> bool {
        std::string p = tmpPath("l1.log");
        std::remove(p.c_str());
        { Logger l(p); l.logCritical("msg1"); }
        std::string s = readFile(p);
        return contains(s, "[CRITICAL]") && contains(s, "msg1");
    });
    run(63, "Запись некритического сообщения", []() -> bool {
        std::string p = tmpPath("l2.log");
        std::remove(p.c_str());
        { Logger l(p); l.logNonCritical("msg2"); }
        std::string s = readFile(p);
        return contains(s, "[NONCRITICAL]") && contains(s, "msg2") && !contains(s, "[CRITICAL]");
    });
    run(64, "Прямой вызов log() с уровнем", []() -> bool {
        std::string p = tmpPath("l3.log");
        std::remove(p.c_str());
        { Logger l(p); l.log(LogLevel::CRITICAL, "x"); }
        return contains(readFile(p), "[CRITICAL] x");
    });
    run(65, "Формат временной метки", []() -> bool {
        std::string p = tmpPath("l4.log");
        std::remove(p.c_str());
        { Logger l(p); l.logCritical("ts"); }
        std::regex re("^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} \\[(CRITICAL|NONCRITICAL)\\] ts\n$");
        return std::regex_match(readFile(p), re);
    });
    run(66, "Несколько записей подряд", []() -> bool {
        std::string p = tmpPath("l5.log");
        std::remove(p.c_str());
        { Logger l(p); l.logCritical("a"); l.logNonCritical("b"); l.logCritical("c"); }
        std::string s = readFile(p);
        return std::count(s.begin(), s.end(), '\n') == 3 && contains(s, "[NONCRITICAL] b") && contains(s, "[CRITICAL] c");
    });
    run(67, "Длинное сообщение", []() -> bool {
        std::string p = tmpPath("l6.log");
        std::remove(p.c_str());
        std::string m(500, 'x');
        { Logger l(p); l.logCritical(m); }
        return contains(readFile(p), m);
    });
    run(68, "Спецсимволы в сообщении", []() -> bool {
        std::string p = tmpPath("l7.log");
        std::remove(p.c_str());
        std::string m = "special: <>&\"'";
        { Logger l(p); l.logCritical(m); }
        return contains(readFile(p), m);
    });
    run(69, "Режим добавления в существующий файл", []() -> bool {
        std::string p = tmpPath("l8.log");
        writeFile(p, "OLD\n");
        { Logger l(p); l.logCritical("new"); }
        std::string s = readFile(p);
        return s.compare(0, 4, "OLD\n") == 0 && contains(s, "new");
    });
    run(70, "Недоступный файл журнала", []() -> bool {
        try { Logger l("/nonexistent_dir/x.log"); } catch (const std::runtime_error&) { return true; }
        return false;
    });
    run(71, "Пустое сообщение", []() -> bool {
        std::string p = tmpPath("l10.log");
        std::remove(p.c_str());
        { Logger l(p); l.logCritical(""); }
        return contains(readFile(p), "[CRITICAL] \n");
    });
    std::cout << "\n[Классы UserInterface и ServerConfig]" << std::endl;
    run(72, "ServerConfig: значения по умолчанию", []() -> bool {
        ServerConfig c;
        return c.getBaseFile() == "/etc/vcalc.conf" && c.getLogFile() == "/var/log/vcalc.log" && c.getPort() == 33333;
    });
    run(73, "ServerConfig: пользовательские значения", []() -> bool {
        ServerConfig c("b.conf", "l.log", 4000);
        return c.getBaseFile() == "b.conf" && c.getLogFile() == "l.log" && c.getPort() == 4000;
    });
    run(74, "Запуск без параметров", []() -> bool {
        return !parseArgs({}, nullptr);
    });
    run(75, "Короткий флаг справки", []() -> bool {
        return !parseArgs({"-h"}, nullptr);
    });
    run(76, "Длинный флаг справки", []() -> bool {
        return !parseArgs({"--help"}, nullptr);
    });
    run(77, "Все параметры (короткие флаги)", []() -> bool {
        ServerConfig c;
        bool ok = parseArgs({"-b", "x.conf", "-l", "x.log", "-p", "9090"}, &c);
        return ok && c.getBaseFile() == "x.conf" && c.getLogFile() == "x.log" && c.getPort() == 9090;
    });
    run(78, "Все параметры (длинные флаги)", []() -> bool {
        ServerConfig c;
        bool ok = parseArgs({"--base", "a.conf", "--log", "a.log", "--port", "44444"}, &c);
        return ok && c.getBaseFile() == "a.conf" && c.getLogFile() == "a.log" && c.getPort() == 44444;
    });
    run(79, "Значения по умолчанию для необязательных параметров", []() -> bool {
        ServerConfig c;
        bool ok = parseArgs({"-p", "8080"}, &c);
        return ok && c.getPort() == 8080 && c.getBaseFile() == "/etc/vcalc.conf" && c.getLogFile() == "/var/log/vcalc.log";
    });
    run(80, "Порт на нижней границе", []() -> bool {
        ServerConfig c;
        bool ok = parseArgs({"-p", "1"}, &c);
        return ok && c.getPort() == 1;
    });
    run(81, "Порт на верхней границе", []() -> bool {
        ServerConfig c;
        bool ok = parseArgs({"-p", "65535"}, &c);
        return ok && c.getPort() == 65535;
    });
    run(82, "Порт 0 вне диапазона", []() -> bool {
        return !parseArgs({"-p", "0"}, nullptr);
    });
    run(83, "Порт 65536 вне диапазона", []() -> bool {
        return !parseArgs({"-p", "65536"}, nullptr);
    });
    run(84, "Отрицательный порт", []() -> bool {
        return !parseArgs({"--port=-5"}, nullptr);
    });
    run(85, "Нечисловое значение порта", []() -> bool {
        return !parseArgs({"-p", "abc"}, nullptr);
    });
    run(86, "Флаг порта без значения", []() -> bool {
        return !parseArgs({"-p"}, nullptr);
    });
    run(87, "Неизвестный параметр", []() -> bool {
        return !parseArgs({"--foo"}, nullptr);
    });
    run(88, "Справка вместе с другими параметрами", []() -> bool {
        return !parseArgs({"-p", "9090", "-h"}, nullptr);
    });

    auto t1 = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(t1 - t0).count();

    // очистка временных файлов
    for (const char* n : {"b1.conf","b2.conf","b3.conf","b4.conf","b5.conf","b6.conf","b7.conf","b8.conf",
                          "b9.conf","b10.conf","b11.conf","b12a.conf","b12b.conf","b13.conf","b14.conf",
                          "l1.log","l2.log","l3.log","l4.log","l5.log","l6.log","l7.log","l8.log","l10.log"})
        std::remove(tmpPath(n).c_str());

    std::printf("\n========================================\n");
    std::printf("РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ\n");
    std::printf("========================================\n");
    std::printf("Количество выполненных тестов: %d\n", g_total);
    std::printf("Успешно пройдено:              %d\n", g_passed);
    std::printf("Завершено с ошибкой:           %d\n", g_failed);
    std::printf("Затраченное время:             %.3f сек\n", sec);
    std::printf("========================================\n");
    return g_failed == 0 ? 0 : 1;
}
