return {
	-- example usage:
	--
	-- ["ex<T>"] = { "ex<double>", "ex<string>" }
	-- module = { ["ex2<K,V>"] = { "example<char*, int>" } }
	
	qtcore = {
		["QList<T>"] = { "QList<QString>", "QList<QFileInfo>", "QList<QVariant>",
			"QList<QModelIndex>", "QList<int>", "QList<QUrl>" },
	},
	qtgui = {
		["QList<T>"] = { "QList<QGraphicsItem*>", "QList<int>", "QList<qreal>",
			"QList<QModelIndex>", "QList<QSize>", "QList<QPolygonF>", "QList<QKeySequence>",
			"QList<QUrl>", "QList<QRectF>", "QList<QImageTextKeyLang>", "QList<QTableWidgetItem*>" },
		["QVector<T>"] = { "QVector<QPointF>", "QVector<QPoint>", "QVector<QRgb>", "QVector<QLine>",
			"QVector<QRectF>", "QVector<QRect>", "QVector<QTextLength>", "QVector<QGradientStop>",
			"QVector<qreal>", "QVector<QColor>", "QVector<QTextFormat>", "QVector<QLineF>" },
	},
	qtnetwork = {
		["QList<T>"] = { "QList<QSslError>", "QList<QSslCertificate>", "QList<QNetworkCookie>",
			"QList<QSslCipher>", "QList<QNetworkAddressEntry>", "QList<QNetworkProxy>", 
			"QList<QHostAddress>", "QList<QUrl>", "QList<QModelIndex>" }
	},
	qtsql = {
		["QList<T>"] = { "QList<QModelIndex>", "QList<QUrl>" },
		["QVector<T>"] = { "QVector<QVariant>" },
	},
}
