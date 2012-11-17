#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

class MyDialog : public QWidget
{
    Q_OBJECT
public:
    MyDialog()
    {
        QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
        vBoxLayout->setMargin(0);
        vBoxLayout->setSpacing(0);

        label = new QLabel(this);
        label->setWordWrap(true);
        vBoxLayout->addWidget(label);

        QPushButton* okButton = new QPushButton(this);
        okButton->setText(tr("Ok"));
        okButton->setMinimumSize(100,60);
        okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QWidget* okButtonParentWidget = new QWidget(this);
        QHBoxLayout* okButtonLayout = new QHBoxLayout(okButtonParentWidget);
        okButtonLayout->setMargin(0); okButtonLayout->setSpacing(0);
        okButtonLayout->addWidget(okButton);

        vBoxLayout->addWidget(okButtonParentWidget);

        connect(okButton, SIGNAL(clicked()), this, SLOT(hideMyDialog()));

    }

    void setText( const QString& text )
    {
        label->setText( text );
    }

public slots:
	void hideMyDialog(){
		this->hide();
		emit dialogHidden();
	}

signals:
	void dialogHidden();

private:
    QLabel* label;
};


#endif // MYDIALOG_H
