#include "core/UiaSwitcher.h"

#include <QDebug>
#include <QDir>
#include <QStringList>

#include <uiautomation.h>
#include <objbase.h>

namespace {

// Nombres de boton "aceptar" aceptados, en varios idiomas/apps.
const QStringList &acceptButtonNames()
{
    static const QStringList names = {
        QStringLiteral("open"), QStringLiteral("save"), QStringLiteral("choose"),
        QStringLiteral("select"), QStringLiteral("abrir"), QStringLiteral("guardar"),
    };
    return names;
}

QString bstrToQString(BSTR bstr)
{
    if (!bstr) {
        return QString();
    }
    return QString::fromWCharArray(bstr, static_cast<int>(SysStringLen(bstr)));
}

} // namespace

namespace UiaSwitcher {

bool switchQtDialog(HWND dlg, const QString &folder)
{
    if (!dlg || folder.isEmpty()) {
        return false;
    }

    IUIAutomation *automation = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_IUIAutomation, reinterpret_cast<void **>(&automation));
    if (FAILED(hr) || !automation) {
        qWarning() << "[UiaSwitcher] No se pudo crear IUIAutomation, hr=" << hr;
        return false;
    }

    IUIAutomationElement *element = nullptr;
    hr = automation->ElementFromHandle(dlg, &element);
    if (FAILED(hr) || !element) {
        qWarning() << "[UiaSwitcher] ElementFromHandle fallo, hr=" << hr;
        automation->Release();
        return false;
    }

    bool result = false;

    // --- Buscar todos los Edit descendientes ---
    VARIANT varEditType;
    VariantInit(&varEditType);
    varEditType.vt = VT_I4;
    varEditType.lVal = UIA_EditControlTypeId;

    IUIAutomationCondition *editCondition = nullptr;
    hr = automation->CreatePropertyCondition(UIA_ControlTypePropertyId, varEditType, &editCondition);
    VariantClear(&varEditType);

    IUIAutomationElementArray *editArray = nullptr;
    int editCount = 0;
    if (SUCCEEDED(hr) && editCondition) {
        hr = element->FindAll(TreeScope_Descendants, editCondition, &editArray);
        if (SUCCEEDED(hr) && editArray) {
            editArray->get_Length(&editCount);
        }
    }
    qDebug() << "[UiaSwitcher] Edit encontrados:" << editCount;

    if (editCount <= 0) {
        qWarning() << "[UiaSwitcher] No se encontro ningun Edit en el dialogo.";
        if (editCondition) editCondition->Release();
        if (editArray) editArray->Release();
        element->Release();
        automation->Release();
        return false;
    }

    // Elegir el edit destino: el que su valor actual contenga '/' o '\\'; si hay
    // varios, el ultimo; si ninguno, el ultimo Edit de la lista.
    IUIAutomationElement *targetEdit = nullptr;
    IUIAutomationElement *lastEdit = nullptr;
    QString targetPreviousValue;

    for (int i = 0; i < editCount; ++i) {
        IUIAutomationElement *editElem = nullptr;
        if (FAILED(editArray->GetElement(i, &editElem)) || !editElem) {
            continue;
        }

        QString currentValue;
        IUIAutomationValuePattern *valuePattern = nullptr;
        if (SUCCEEDED(editElem->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern,
                                                      reinterpret_cast<void **>(&valuePattern))) &&
            valuePattern) {
            BSTR bstrVal = nullptr;
            if (SUCCEEDED(valuePattern->get_CurrentValue(&bstrVal))) {
                currentValue = bstrToQString(bstrVal);
                if (bstrVal) SysFreeString(bstrVal);
            }
            valuePattern->Release();
        }

        if (lastEdit) {
            lastEdit->Release();
        }
        lastEdit = editElem;
        lastEdit->AddRef();

        if (currentValue.contains(QLatin1Char('/')) || currentValue.contains(QLatin1Char('\\'))) {
            if (targetEdit) {
                targetEdit->Release();
            }
            targetEdit = editElem;
            targetEdit->AddRef();
            targetPreviousValue = currentValue;
        }

        editElem->Release();
    }

    if (!targetEdit) {
        // Ninguno tenia separador: usar el ultimo Edit de la lista.
        targetEdit = lastEdit;
        if (targetEdit) {
            targetEdit->AddRef();
        }
    }
    if (lastEdit) {
        lastEdit->Release();
        lastEdit = nullptr;
    }

    editCondition->Release();
    editArray->Release();

    if (!targetEdit) {
        qWarning() << "[UiaSwitcher] No se pudo elegir un Edit destino.";
        element->Release();
        automation->Release();
        return false;
    }

    // El path ya viene normalizado desde DialogSwitcher::switchDialog
    // (barras invertidas y barra final). No se re-normaliza aca.
    const QString &path = folder;

    IUIAutomationValuePattern *targetValuePattern = nullptr;
    if (SUCCEEDED(targetEdit->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern,
                                                    reinterpret_cast<void **>(&targetValuePattern))) &&
        targetValuePattern) {
        BSTR bstrPath = SysAllocString(reinterpret_cast<const wchar_t *>(path.utf16()));
        HRESULT setHr = targetValuePattern->SetValue(bstrPath);
        if (bstrPath) SysFreeString(bstrPath);
        targetValuePattern->Release();

        if (SUCCEEDED(setHr)) {
            result = true;
            qDebug() << "[UiaSwitcher] Valor previo:" << targetPreviousValue
                      << "-> escrito:" << path;
        } else {
            qWarning() << "[UiaSwitcher] SetValue fallo, hr=" << setHr;
        }
    } else {
        qWarning() << "[UiaSwitcher] El Edit elegido no soporta ValuePattern.";
    }

    targetEdit->Release();

    if (!result) {
        element->Release();
        automation->Release();
        return false;
    }

    // --- Buscar boton de aceptar e invocar ---
    VARIANT varButtonType;
    VariantInit(&varButtonType);
    varButtonType.vt = VT_I4;
    varButtonType.lVal = UIA_ButtonControlTypeId;

    IUIAutomationCondition *buttonCondition = nullptr;
    hr = automation->CreatePropertyCondition(UIA_ControlTypePropertyId, varButtonType, &buttonCondition);
    VariantClear(&varButtonType);

    bool invoked = false;
    if (SUCCEEDED(hr) && buttonCondition) {
        IUIAutomationElementArray *buttonArray = nullptr;
        hr = element->FindAll(TreeScope_Descendants, buttonCondition, &buttonArray);
        if (SUCCEEDED(hr) && buttonArray) {
            int buttonCount = 0;
            buttonArray->get_Length(&buttonCount);
            for (int i = 0; i < buttonCount && !invoked; ++i) {
                IUIAutomationElement *btn = nullptr;
                if (FAILED(buttonArray->GetElement(i, &btn)) || !btn) {
                    continue;
                }
                BSTR bstrName = nullptr;
                QString name;
                if (SUCCEEDED(btn->get_CurrentName(&bstrName))) {
                    name = bstrToQString(bstrName);
                    if (bstrName) SysFreeString(bstrName);
                }
                if (acceptButtonNames().contains(name.trimmed(), Qt::CaseInsensitive)) {
                    IUIAutomationInvokePattern *invokePattern = nullptr;
                    if (SUCCEEDED(btn->GetCurrentPatternAs(UIA_InvokePatternId, IID_IUIAutomationInvokePattern,
                                                            reinterpret_cast<void **>(&invokePattern))) &&
                        invokePattern) {
                        HRESULT invHr = invokePattern->Invoke();
                        invokePattern->Release();
                        if (SUCCEEDED(invHr)) {
                            invoked = true;
                            qDebug() << "[UiaSwitcher] Boton invocado:" << name;
                        } else {
                            qWarning() << "[UiaSwitcher] Invoke fallo en boton" << name << "hr=" << invHr;
                        }
                    }
                }
                btn->Release();
            }
            buttonArray->Release();
        }
        buttonCondition->Release();
    }

    if (!invoked) {
        qDebug() << "[UiaSwitcher] No se encontro/invoco boton de aceptar; valor queda escrito.";
    }

    element->Release();
    automation->Release();
    return true;
}

} // namespace UiaSwitcher
