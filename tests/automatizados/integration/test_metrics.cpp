#include <gtest/gtest.h>
#include <httplib.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include "http/handler_metrics.hpp"

class MetricsTest : public ::testing::Test {
protected:
    std::string dbPath = "test_metrics.db";
    pulso::storage::Storage* storage = nullptr;
    httplib::Server server;
    std::thread serverThread;
    int port = 18080;

    void SetUp() override {
        std::remove(dbPath.c_str());
        storage = new pulso::storage::Storage(dbPath);

        pulso::core::Snapshot snapshot;
        snapshot.timestamp = 1234567890;

        pulso::core::Metrica metrica;
        metrica.nombre = "cpu.usage";
        metrica.valor = 45.5;
        metrica.unidad = "percent";
        metrica.timestamp = snapshot.timestamp;
        snapshot.metricas.push_back(metrica);

        storage->save(snapshot);
        pulso::http::HandleMetrics(server, *storage);

        serverThread = std::thread([this]() {
            server.listen("127.0.0.1", port);
        });

        for (int i = 0; i < 50 && !server.is_running(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    void TearDown() override {
        server.stop();

        if (serverThread.joinable()) {
            serverThread.join();
        }

        delete storage;
        storage = nullptr;

        std::remove(dbPath.c_str());
        std::remove((dbPath + "-wal").c_str());
        std::remove((dbPath + "-shm").c_str());
    }
};

TEST_F(MetricsTest, GetMetricsReturns200) {
    httplib::Client client("127.0.0.1", port);
    auto response = client.Get("/metrics");

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->status, 200);
    EXPECT_EQ(response->get_header_value("Content-Type"), "application/json");
    EXPECT_NE(response->body.find("timestamp"), std::string::npos);
    EXPECT_NE(response->body.find("metricas"), std::string::npos);
    EXPECT_NE(response->body.find("cpu.usage"), std::string::npos);
    EXPECT_NE(response->body.find("45.5"), std::string::npos);
}
