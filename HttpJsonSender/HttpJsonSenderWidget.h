#ifndef HTTPJSONSENDERWIDGET_H
#define HTTPJSONSENDERWIDGET_H

#pragma once
#include <QWidget>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QByteArray>
#include <QVBoxLayout>
#include <QLabel>


class HttpJsonSenderWidget : public QWidget
{
	Q_OBJECT
public:
	HttpJsonSenderWidget(QWidget* parent = nullptr);
private slots:
	void sendJsonData();

private:
	QNetworkAccessManager* networkManager;
	QLabel* statusLabel;
};

#endif // HTTPJSONSENDERWIDGET_H