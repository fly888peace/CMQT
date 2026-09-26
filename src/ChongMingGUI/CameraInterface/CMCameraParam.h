#ifndef CAMERAPARAM_H
#define CAMERAPARAM_H
/**************************************************
 * 相机参数文件，定义了相机所支持的六种参数类型，这六个参数
 * 类型是我们重明用来统一各种品牌相机参数的参数抽象层。
 * 因为各个品牌相机对相机参数类的实现是各不相同的，所以我们需要自己实现一个统一的
 * 相机参数类型。
 *
 * 复刻说明：基本保持源工程设计（QVariant 值语义 + 访问位域），
 * 仅给只读成员函数补 `const`（isValid/isReadable/isWriteable）。
 **************************************************/

#include <QString>
#include <QVariant>
#include <QVector>

enum CMParamType {
    UNKNOWN = 0,
    INT,
    DOUBLE,
    ENUM,
    BOOL,
    CMD,
    STRING
};

struct CameraParamMetaInfo {
    QString group;
    QString name;
    CMParamType type { UNKNOWN };
    QString relative_list;
    QString tips;
};

struct CMParam {
    virtual ~CMParam() = default;
    virtual CMParam* clone() = 0;
};

struct IntParam : public CMParam {
    int64_t value {};
    int64_t min {};
    int64_t max {};
    int64_t increment {};

    CMParam* clone() override
    {
        return new IntParam(*this);
    }
};

struct DoubleParam : public CMParam {
    double value {};
    double min {};
    double max {};

    CMParam* clone() override
    {
        return new DoubleParam(*this);
    }
};

struct BoolParam : public CMParam {
    bool value {};

    CMParam* clone() override
    {
        return new BoolParam(*this);
    }
};

struct StringParam : public CMParam {
    QString value {};
    unsigned int nMaxLength {};

    CMParam* clone() override
    {
        return new StringParam(*this);
    }
};

struct EnumParam : public CMParam {
    QString value {};
    QVector<QString> availableValue;
    int valueInt {};
    QVector<int> availableInt;

    CMParam* clone() override
    {
        return new EnumParam(*this);
    }
};

struct CmdParam : public CMParam {
    CMParam* clone() override
    {
        return new CmdParam(*this);
    }
};

class CameraParam {
public:
    CameraParam()
    {
    }

    CameraParam(CameraParamMetaInfo meta)
        : _meta(meta)
    {
    }

    CameraParam(const CameraParam& param)
    {
        _meta = param._meta;
        _value = param._value;
        _accessMode = param._accessMode;
    }

    QVariant GetValue() const
    {
        return _value;
    }

    void SetValue(const QVariant& value)
    {
        _value = value;
    }

    void reset(CameraParamMetaInfo meta)
    {
        _meta = meta;
    }

    const QString& name() const
    {
        return _meta.name;
    }

    const QString& group() const
    {
        return _meta.group;
    }

    const QString& relativeList() const
    {
        return _meta.relative_list;
    }

    CMParamType type() const
    {
        return _meta.type;
    }

    const QString& tips() const
    {
        return _meta.tips;
    }

    QString displayText() const
    {
        switch (type()) {
        case STRING: {
            StringParam varParam = GetValue().value<StringParam>();
            return varParam.value;
        }
        case CMD: {
            return "{Commond}";
        }
        case INT: {
            IntParam varParam = GetValue().value<IntParam>();
            return QString::number(varParam.value);
        }
        case DOUBLE: {
            DoubleParam varParam = GetValue().value<DoubleParam>();
            return QString::number(varParam.value);
        }
        case BOOL: {
            BoolParam varParam = GetValue().value<BoolParam>();
            return varParam.value ? "True" : "False";
        }
        case ENUM: {
            EnumParam varParam = GetValue().value<EnumParam>();
            return varParam.value;
        }
        default:
            break;
        }
        return QString("unknow");
    }

    bool isValid() const
    {
        return _accessMode.valid;
    }
    bool isReadable() const
    {
        return _accessMode.readable;
    }
    bool isWriteable() const
    {
        return _accessMode.writeable;
    }

    void setValid(bool valid)
    {
        _accessMode.valid = valid;
    }
    void setReadable(bool readable)
    {
        _accessMode.readable = readable;
    }
    void setWriteable(bool writeable)
    {
        _accessMode.writeable = writeable;
    }

private:
    CameraParamMetaInfo _meta;
    QVariant _value;
    struct
    {
        bool valid : 1;
        bool readable : 1;
        bool writeable : 1;
    } _accessMode {};
};

// 自定义类型注册
// Qt6以前需要手动注册自定义类型，才能使用，Qt6可以自动识别
Q_DECLARE_METATYPE(IntParam);
Q_DECLARE_METATYPE(DoubleParam);
Q_DECLARE_METATYPE(BoolParam);
Q_DECLARE_METATYPE(StringParam);
Q_DECLARE_METATYPE(EnumParam);
Q_DECLARE_METATYPE(CmdParam);
Q_DECLARE_METATYPE(CameraParam);

#endif
