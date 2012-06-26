/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "xmlparser.hpp"

#include <ClanLib/Display/Image/pixel_buffer.h>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <platform.hpp>
#include <misc/log.hpp>
#include <client.hpp>

#include "manager.hpp"
#include "texture.hpp"
#include "components/templatebutton.hpp"
#include "components/uobutton.hpp"
#include "components/scrollarea.hpp"
#include "components/lineedit.hpp"
#include "components/label.hpp"
#include "components/clicklabel.hpp"
#include "components/worldview.hpp"
#include "components/propertylabel.hpp"
#include "components/paperdollview.hpp"
#include "components/containerview.hpp"
#include "components/image.hpp"
#include "components/background.hpp"
#include "components/uocheckbox.hpp"
#include "components/sysloglabel.hpp"
#include "components/tbackground.hpp"
#include "components/warmodebutton.hpp"

namespace fluo {
namespace ui {

XmlParser* XmlParser::singleton_ = NULL;

XmlParser* XmlParser::getSingleton() {
    if (!singleton_) {
        singleton_ = new XmlParser();
    }

    return singleton_;
}

XmlParser::XmlParser() {
    functionTable_["tbutton"] = boost::bind(&XmlParser::parseTButton, this, _1, _2, _3);
    functionTable_["tcheckbox"] = boost::bind(&XmlParser::parseTCheckBox, this, _1, _2, _3);
    functionTable_["tradiobutton"] = boost::bind(&XmlParser::parseTRadioButton, this, _1, _2, _3);
    functionTable_["tlineedit"] = boost::bind(&XmlParser::parseTLineEdit, this, _1, _2, _3);
    functionTable_["tcombobox"] = boost::bind(&XmlParser::parseTComboBox, this, _1, _2, _3);
    functionTable_["tgroupbox"] = boost::bind(&XmlParser::parseTGroupBox, this, _1, _2, _3);
    functionTable_["tspin"] = boost::bind(&XmlParser::parseTSpin, this, _1, _2, _3);
    functionTable_["ttabs"] = boost::bind(&XmlParser::parseTTabs, this, _1, _2, _3);
    functionTable_["tslider"] = boost::bind(&XmlParser::parseTSlider, this, _1, _2, _3);
    functionTable_["tlabel"] = boost::bind(&XmlParser::parseTLabel, this, _1, _2, _3);
    functionTable_["tclicklabel"] = boost::bind(&XmlParser::parseTClickLabel, this, _1, _2, _3);
    functionTable_["ttextedit"] = boost::bind(&XmlParser::parseTTextEdit, this, _1, _2, _3);
    functionTable_["tscrollarea"] = boost::bind(&XmlParser::parseTScrollArea, this, _1, _2, _3);
    functionTable_["repeat"] = boost::bind(&XmlParser::parseRepeat, this, _1, _2, _3);
    functionTable_["propertylabel"] = boost::bind(&XmlParser::parsePropertyLabel, this, _1, _2, _3);
    functionTable_["tbackground"] = boost::bind(&XmlParser::parseTBackground, this, _1, _2, _3);

    functionTable_["page"] = boost::bind(&XmlParser::parsePage, this, _1, _2, _3);
    functionTable_["background"] = boost::bind(&XmlParser::parseBackground, this, _1, _2, _3);
    functionTable_["button"] = boost::bind(&XmlParser::parseButton, this, _1, _2, _3);
    functionTable_["checkbox"] = boost::bind(&XmlParser::parseCheckbox, this, _1, _2, _3);

    functionTable_["image"] = boost::bind(&XmlParser::parseImage, this, _1, _2, _3);
    functionTable_["worldview"] = boost::bind(&XmlParser::parseWorldView, this, _1, _2, _3);
    functionTable_["paperdoll"] = boost::bind(&XmlParser::parsePaperdoll, this, _1, _2, _3);
    functionTable_["container"] = boost::bind(&XmlParser::parseContainer, this, _1, _2, _3);
    functionTable_["sysloglabel"] = boost::bind(&XmlParser::parseSysLogLabel, this, _1, _2, _3);
    functionTable_["warmodebutton"] = boost::bind(&XmlParser::parseWarModeButton, this, _1, _2, _3);
}

void XmlParser::addRepeatContext(const UnicodeString& name, const RepeatContext& context) {
    getSingleton()->repeatContexts_[name] = context;
}

void XmlParser::removeRepeatContext(const UnicodeString& name) {
    getSingleton()->repeatContexts_.erase(name);
}

void XmlParser::clearRepeatContexts() {
    getSingleton()->repeatContexts_.clear();
}

GumpMenu* XmlParser::fromXmlFile(const UnicodeString& name, GumpMenu* menu) {
    boost::filesystem::path path = "gumps";
    std::string utf8FileName = StringConverter::toUtf8String(name) + ".xml";
    path = path / utf8FileName;

    path = data::Manager::getShardFilePath(path);

    if (!boost::filesystem::exists(path)) {
        LOG_ERROR << "Unable to gump xml, file not found: " << utf8FileName << std::endl;
        return menu;
    }

    LOG_DEBUG << "Parsing xml gump file: " << path << std::endl;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.string().c_str());

    if (result) {
        GumpMenu* ret = getSingleton()->fromXml(doc, menu);

        // save transformed document (debug)
        //LOG_DEBUG << "Saving result: " << doc.save_file("out.xml") << std::endl;
        //LOG_DEBUG << "Transformed xml:" << std::endl;
        //doc.print(std::cout);

        if (ret) {
            ret->setName(name);
        }

        return ret;
    } else {
        LOG_ERROR << "Error parsing gump xml file at offset " << result.offset << ": " << result.description() << std::endl;
        return menu;
    }
}

GumpMenu* XmlParser::fromXmlString(const UnicodeString& str, GumpMenu* menu) {
    pugi::xml_document doc;
    std::string utf8String = StringConverter::toUtf8String(str);
    pugi::xml_parse_result result = doc.load_buffer(utf8String.c_str(), utf8String.length());

    if (result) {
        GumpMenu* ret = getSingleton()->fromXml(doc, menu);

        return ret;
    } else {
        LOG_ERROR << "Error parsing gump xml string at offset " << result.offset << ": " << result.description() << std::endl;
        return menu;
    }
}


GumpMenu* XmlParser::fromXml(pugi::xml_document& doc, GumpMenu* menu) {

    pugi::xml_node rootNode = doc.child("gump");

    GumpMenu* ret = menu;

    if (!ret) {
        CL_Rect bounds = getBoundsFromNode(rootNode);
        bounds.set_width(1);
        bounds.set_height(1);

        bool closable = rootNode.attribute("closable").as_bool();
        bool draggable = rootNode.attribute("draggable").as_bool();
        UnicodeString action = StringConverter::fromUtf8(rootNode.attribute("action").value());
        UnicodeString cancelAction = StringConverter::fromUtf8(rootNode.attribute("cancelaction").value());
        bool inbackground = rootNode.attribute("inbackground").as_bool();

        CL_GUITopLevelDescription desc(bounds, false);
        desc.set_decorations(false);
        desc.set_in_background(inbackground);

        ret = new GumpMenu(desc);
        ret->setClosable(closable);
        ret->setDraggable(draggable);

        if (action.length() > 0) {
            ret->setAction(action);
        }

        if (cancelAction.length() > 0) {
            ret->setCancelAction(cancelAction);
        }
    }

    parseChildren(rootNode, ret, ret);

    ret->fitSizeToChildren();
    ret->setupResizeHandler();
    ret->activateFirstPage();

    return ret;
}

bool XmlParser::parseChildren(pugi::xml_node& rootNode, CL_GUIComponent* parent, GumpMenu* top) {
    pugi::xml_node_iterator iter = rootNode.begin();
    pugi::xml_node_iterator iterEnd = rootNode.end();

    bool ret = true;

    for (; iter != iterEnd && ret; ++iter) {
        std::map<UnicodeString, XmlParseFunction>::iterator function = functionTable_.find(iter->name());

        if (function != functionTable_.end()) {
            ret = (function->second)(*iter, parent, top);
        } else {
            LOG_WARN << "Unknown gump tag " << iter->name() << std::endl;
        }
    }

    return ret;
}

CL_Rect XmlParser::getBoundsFromNode(pugi::xml_node& node) {
    unsigned int width = node.attribute("width").as_uint();
    unsigned int height = node.attribute("height").as_uint();
    int locX = node.attribute("x").as_int();
    int locY = node.attribute("y").as_int();

    CL_Rect bounds(locX, locY, CL_Size(width, height));
    return bounds;
}

bool XmlParser::parseId(pugi::xml_node& node, CL_GUIComponent* component) {
    std::string cssid = node.attribute("id").value();
    if (cssid.length() > 0) {
        component->set_id_name(cssid);
    }

    return true;
}

bool XmlParser::parseTButton(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    UnicodeString text = StringConverter::fromUtf8(node.attribute("text").value());
    unsigned int buttonId = node.attribute("buttonid").as_uint();
    unsigned int pageId = node.attribute("page").as_uint();
    UnicodeString action = StringConverter::fromUtf8(node.attribute("action").value());
    UnicodeString param = StringConverter::fromUtf8(node.attribute("param").value());
    UnicodeString param2 = StringConverter::fromUtf8(node.attribute("param2").value());
    UnicodeString param3 = StringConverter::fromUtf8(node.attribute("param3").value());
    UnicodeString param4 = StringConverter::fromUtf8(node.attribute("param4").value());
    UnicodeString param5 = StringConverter::fromUtf8(node.attribute("param5").value());

    components::TemplateButton* button = new components::TemplateButton(parent);
    if (action.length() > 0) {
        button->setLocalButton(action);
        button->setParameter(param, 0);
        button->setParameter(param2, 1);
        button->setParameter(param3, 2);
        button->setParameter(param4, 3);
        button->setParameter(param5, 4);
    } else if (!node.attribute("buttonid").empty()) {
        button->setServerButton(buttonId);
    } else if (!node.attribute("page").empty()) {
        button->setPageButton(pageId);
    } else {
        LOG_WARN << "Button without action, id or page" << std::endl;
        return false;
    }

    parseId(node, button);
    button->set_geometry(bounds);
    button->setText(text);

    top->addToCurrentPage(button);
    return true;
}

bool XmlParser::parseButton(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    unsigned int buttonId = node.attribute("buttonid").as_uint();
    unsigned int pageId = node.attribute("page").as_uint();
    UnicodeString action = StringConverter::fromUtf8(node.attribute("action").value());
    UnicodeString param = StringConverter::fromUtf8(node.attribute("param").value());
    UnicodeString param2 = StringConverter::fromUtf8(node.attribute("param2").value());
    UnicodeString param3 = StringConverter::fromUtf8(node.attribute("param3").value());
    UnicodeString param4 = StringConverter::fromUtf8(node.attribute("param4").value());
    UnicodeString param5 = StringConverter::fromUtf8(node.attribute("param5").value());

    components::UoButton* button = new components::UoButton(parent);
    if (action.length() > 0) {
        button->setLocalButton(action);
        button->setParameter(param, 0);
        button->setParameter(param2, 1);
        button->setParameter(param3, 2);
        button->setParameter(param4, 3);
        button->setParameter(param5, 4);
    } else if (!node.attribute("buttonid").empty()) {
        button->setServerButton(buttonId);
    } else if (!node.attribute("page").empty()) {
        button->setPageButton(pageId);
    } else {
        LOG_WARN << "Button without action, id or page" << std::endl;
        return false;
    }

    parseId(node, button);

    pugi::xml_node normalNode = node.child("normal");
    pugi::xml_node mouseOverNode = node.child("mouseover");
    pugi::xml_node mouseDownNode = node.child("mousedown");

    if (normalNode) {
        parseMultiTextureImage(normalNode, button, components::UoButton::TEX_INDEX_UP);
    } else {
        LOG_ERROR << "Normal image for uo button not defined" << std::endl;
        node.print(std::cout);
        return false;
    }

    if (mouseOverNode) {
        parseMultiTextureImage(mouseOverNode, button, components::UoButton::TEX_INDEX_MOUSEOVER);
    }
    if (mouseDownNode) {
        parseMultiTextureImage(mouseDownNode, button, components::UoButton::TEX_INDEX_DOWN);
    }

    if (bounds.get_width() == 0 || bounds.get_height() == 0) {
        button->setAutoResize(true);
        bounds.set_width(1);
        bounds.set_height(1);
    }

    button->updateTexture();

    button->set_geometry(bounds);

    top->addToCurrentPage(button);
    return true;
}

bool XmlParser::parseTCheckBox(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string text = node.attribute("text").value();
    int checked = node.attribute("checked").as_int();

    CL_CheckBox* cb = new CL_CheckBox(parent);
    parseId(node, cb);
    cb->set_geometry(bounds);
    cb->set_text(text);

    if (checked) {
        cb->set_checked(true);
    }

    top->addToCurrentPage(cb);
    return true;
}

bool XmlParser::parseTRadioButton(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string text = node.attribute("text").value();
    std::string group = node.attribute("group").value();
    int selected = node.attribute("selected").as_int();

    if (group.length() == 0) {
        LOG_ERROR << "Adding tradiobutton without group" << std::endl;
        return false;
    }

    CL_RadioButton* button = new CL_RadioButton(parent);
    parseId(node, button);
    button->set_geometry(bounds);
    button->set_text(text);
    button->set_group_name(group);

    if (selected) {
        button->set_selected(true);
    }

    top->addToCurrentPage(button);
    return true;
}

bool XmlParser::parseTLineEdit(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    UnicodeString text = StringConverter::fromUtf8(node.attribute("text").value());
    int numeric = node.attribute("numeric").as_int();
    int password = node.attribute("password").as_int();
    unsigned int maxlength = node.attribute("maxlength").as_uint();
    UnicodeString action = StringConverter::fromUtf8(node.attribute("action").value());

    components::LineEdit* edit = new components::LineEdit(parent);
    parseId(node, edit);
    edit->setText(text);
    edit->set_geometry(bounds);
    edit->set_select_all_on_focus_gain(false);

    if (password) {
        edit->set_password_mode(true);
    }

    if (numeric) {
        edit->set_numeric_mode(true);
    }

    if (maxlength) {
        edit->set_max_length(maxlength);
    }

    if (action.length() > 0) {
        edit->setAction(action);
    }

    top->addToCurrentPage(edit);
    return true;
}

bool XmlParser::parseTComboBox(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    CL_ComboBox* box = new CL_ComboBox(parent);
    parseId(node, box);
    box->set_geometry(bounds);

    CL_PopupMenu menu;

    pugi::xml_node_iterator iter = node.begin();
    pugi::xml_node_iterator iterEnd = node.end();

    unsigned int selected = 0;

    for (unsigned int index = 0; iter != iterEnd; ++iter, ++index) {
        if (strcmp(iter->name(), "option") != 0) {
            LOG_WARN << "Something different than option in combobox: " << iter->name() << std::endl;
            return false;
        } else {
            std::string text = iter->attribute("text").value();
            int isSelected = iter->attribute("selected").as_int();

            menu.insert_item(text);
            if (isSelected) {
                selected = index;
            }
        }
    }

    box->set_popup_menu(menu);
    box->set_selected_item(selected);

    top->addToCurrentPage(box);
    return true;
}

bool XmlParser::parseTGroupBox(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    CL_GroupBox* box = new CL_GroupBox(parent);
    parseId(node, box);
    box->set_geometry(bounds);

    parseChildren(node, box, top);

    top->addToCurrentPage(box);
    return true;
}

bool XmlParser::parseTSpin(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string type = node.attribute("type").value();

    CL_Spin* spin = new CL_Spin(parent);
    parseId(node, spin);

    if (type == "int") {
        int min = node.attribute("min").as_int();
        int max = node.attribute("max").as_int();
        unsigned int stepsize = node.attribute("stepsize").as_uint();
        int value = node.attribute("value").as_int();

        spin->set_floating_point_mode(false);
        spin->set_ranges(min, max);
        spin->set_step_size(stepsize);
        spin->set_value(value);
    } else if (type == "float") {
        float min = node.attribute("min").as_float();
        float max = node.attribute("max").as_float();
        float stepsize = node.attribute("stepsize").as_float();
        float value = node.attribute("value").as_float();

        spin->set_floating_point_mode(true);
        spin->set_ranges_float(min, max);
        spin->set_step_size_float(stepsize);
        spin->set_value_float(value);
    } else {
        LOG_WARN << "Unknown spin type: " << type << std::endl;
        return false;
    }

    spin->set_geometry(bounds);

    top->addToCurrentPage(spin);
    return true;
}

bool XmlParser::parseTTabs(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    CL_Tab* tabs = new CL_Tab(parent);
    parseId(node, tabs);
    tabs->set_geometry(bounds);

    pugi::xml_node_iterator iter = node.begin();
    pugi::xml_node_iterator iterEnd = node.end();

    for (; iter != iterEnd; ++iter) {
        if (strcmp(iter->name(), "ttabpage") != 0) {
            LOG_WARN << "Something different than ttabpage in ttabs: " << iter->name() << std::endl;
            return false;
        } else {
            std::string tabTitle = iter->attribute("text").value();
            CL_TabPage* newpage = tabs->add_page(tabTitle);
            parseId(*iter, newpage);

            parseChildren(*iter, newpage, top);
        }
    }

    top->addToCurrentPage(tabs);
    return true;
}

bool XmlParser::parseTSlider(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string type = node.attribute("type").value();
    int min = node.attribute("min").as_int();
    int max = node.attribute("max").as_int();
    unsigned int pagestep = node.attribute("pagestep").as_uint();
    int value = node.attribute("value").as_int();


    CL_Slider* slider = new CL_Slider(parent);
    parseId(node, slider);

    if (type == "vertical") {
        slider->set_vertical(true);
    } else if (type == "horizontal") {
        slider->set_horizontal(true);
    } else {
        LOG_WARN << "Unknown slider type: " << type << std::endl;
        return false;
    }

    slider->set_min(min);
    slider->set_max(max);
    slider->set_page_step(pagestep);
    slider->set_position(value);
    slider->set_lock_to_ticks(false);
    slider->set_geometry(bounds);

    top->addToCurrentPage(slider);
    return true;
}

bool XmlParser::parseTLabel(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string align = node.attribute("align").value();
    UnicodeString text = node.attribute("text").value();

    components::Label* label = new components::Label(parent);
    parseId(node, label);

    if (align.length() == 0 || align == "left") {
        label->set_alignment(CL_Label::align_left);
    } else if (align == "right") {
        label->set_alignment(CL_Label::align_right);
    } else if (align == "center") {
        label->set_alignment(CL_Label::align_center);
    } else if (align == "justify") {
        label->set_alignment(CL_Label::align_justify);
    } else {
        LOG_WARN << "Unknown label align: " << align << std::endl;
        return false;
    }

    label->setText(text);
    label->set_geometry(bounds);

    top->addToCurrentPage(label);
    return true;
}

bool XmlParser::parsePropertyLabel(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string align = node.attribute("align").value();
    UnicodeString link = StringConverter::fromUtf8(node.attribute("link").value());

    if (link.length() == 0) {
        LOG_WARN << "PropertyLabel without link" << std::endl;
        return false;
    }

    components::PropertyLabel* label = new components::PropertyLabel(parent, link);
    parseId(node, label);

    if (align.length() == 0 || align == "left") {
        label->set_alignment(CL_Label::align_left);
    } else if (align == "right") {
        label->set_alignment(CL_Label::align_right);
    } else if (align == "center") {
        label->set_alignment(CL_Label::align_center);
    } else if (align == "justify") {
        label->set_alignment(CL_Label::align_justify);
    } else {
        LOG_WARN << "Unknown label align: " << align << std::endl;
        return false;
    }

    label->set_geometry(bounds);

    top->addToCurrentPage(label);
    return true;
}

bool XmlParser::parseTTextEdit(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    // does not seem to work currently (clanlib problem)

    //CL_Rect bounds = getBoundsFromNode(node);
    //std::string text = node.attribute("text").value();

    //CL_GUIComponentDescription desc;
    //std::string cssid = node.attribute("cssid").value();
    //desc.set_type_name("TextEdit");
    //if (cssid.length() > 0) {
        //desc.set_id_name(cssid);
    //}
    //CL_TextEdit* edit = new CL_TextEdit(desc, parent);
    //edit->set_geometry(bounds);

    //return true;

    LOG_WARN << "TextEdit is currently not supported, sorry!" << std::endl;
    return false;
}

bool XmlParser::parseImage(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    UnicodeString imgSource = StringConverter::fromUtf8(node.attribute("source").value());
    UnicodeString imgId = StringConverter::fromUtf8(node.attribute("imgid").value());

    CL_Rect bounds = getBoundsFromNode(node);

    unsigned int hue = node.attribute("hue").as_uint();
    std::string rgba = node.attribute("rgba").value();
    float alpha = node.attribute("alpha").as_float();

    bool tiled = node.attribute("tiled").as_bool();

    components::Image* img = new components::Image(parent);
    parseId(node, img);

    boost::shared_ptr<ui::Texture> texture = data::Manager::getTexture(imgSource, imgId);
    texture->setUsage(ui::Texture::USAGE_GUMP);

    if (!texture) {
        LOG_ERROR << "Unable to parse gump image, source=" << imgSource << " imgid=" << imgId << std::endl;
        return false;
    }

    if (bounds.get_width() == 0 || bounds.get_height() == 0) {
        if (texture->getWidth() != 1) {
            bounds.set_width(texture->getWidth());
            bounds.set_height(texture->getHeight());
        } else {
            img->setAutoResize(true);
            bounds.set_width(1);
            bounds.set_height(1);
        }
    } else if (tiled) {
        img->setTiled(true);
    }

    img->setTexture(texture);
    img->set_geometry(bounds);

    img->setHue(hue);
    if (rgba.length() > 0) {
        img->setColorRGBA(CL_Colorf(rgba));
    }

    if (alpha) {
        img->setAlpha(alpha);
    }

    top->addToCurrentPage(img);
    return true;
}

bool XmlParser::parseBackground(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    unsigned int hue = node.attribute("hue").as_uint();
    std::string rgba = node.attribute("rgba").value();
    float alpha = node.attribute("alpha").as_float();

    unsigned int gumpId = node.attribute("gumpid").as_uint();

    if (!gumpId) {
        LOG_ERROR << "Unable to parse background, gumpid not found, not a number or zero" << std::endl;
        return false;
    }

    components::Background* img = new components::Background(parent);
    parseId(node, img);

    img->set_geometry(bounds);

    img->setBaseId(gumpId);

    img->setHue(hue);
    if (rgba.length() > 0) {
        img->setColorRGBA(CL_Colorf(rgba));
    }

    if (alpha) {
        img->setAlpha(alpha);
    }

    top->addToCurrentPage(img);
    return true;
}

bool XmlParser::parsePage(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    unsigned int number = node.attribute("number").as_uint();

    if (top->getActivePageId() != 0) {
        // check that we add pages only at the top level hierarchy
        // adding a page inside another page
        LOG_ERROR << "Adding page " << top->getActivePageId() << " inside of page " << number << std::endl;
        return false;
    }

    top->addPage(number);

    bool ret = parseChildren(node, parent, top);

    top->activatePage(0);

    return ret;
}

bool XmlParser::parseTScrollArea(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string hVisibilityStr = node.attribute("hvisible").value();
    std::string vVisibilityStr = node.attribute("vvisible").value();

    static std::string visibilityAlways("always");
    static std::string visibilityNever("never");
    static std::string visibilityOnDemand("ondemand");

    unsigned int hVisibility = components::ScrollArea::VISIBLE_ON_DEMAND;
    if (hVisibilityStr.length() > 0) {
        if (hVisibilityStr == visibilityAlways) {
            hVisibility = components::ScrollArea::VISIBLE_ALWAYS;
        } else if (hVisibilityStr == visibilityNever) {
            hVisibility = components::ScrollArea::VISIBLE_NEVER;
        } else if (hVisibilityStr == visibilityOnDemand) {
            hVisibility = components::ScrollArea::VISIBLE_ON_DEMAND;
        } else {
            LOG_ERROR << "Unknown scrollbar hvisibility: " << hVisibilityStr << ". Possible values: always/never/ondemand" << std::endl;
            return false;
        }
    }

    unsigned int vVisibility = components::ScrollArea::VISIBLE_ON_DEMAND;
    if (vVisibilityStr.length() > 0) {
        if (vVisibilityStr == visibilityAlways) {
            vVisibility = components::ScrollArea::VISIBLE_ALWAYS;
        } else if (vVisibilityStr == visibilityNever) {
            vVisibility = components::ScrollArea::VISIBLE_NEVER;
        } else if (vVisibilityStr == visibilityOnDemand) {
            vVisibility = components::ScrollArea::VISIBLE_ON_DEMAND;
        } else {
            LOG_ERROR << "Unknown scrollbar vvisibility: " << vVisibilityStr << ". Possible values: always/never/ondemand" << std::endl;
            return false;
        }
    }


    components::ScrollArea* area = new components::ScrollArea(parent);
    parseId(node, area);
    area->set_geometry(bounds);

    unsigned int hLineStep = node.attribute("hstep").as_uint();
    unsigned int hPageStep = node.attribute("hpage").as_uint();
    unsigned int vLineStep = node.attribute("vstep").as_uint();
    unsigned int vPageStep = node.attribute("vpage").as_uint();
    unsigned int marginLeft = node.attribute("marginleft").as_uint();
    unsigned int marginBottom = node.attribute("marginbottom").as_uint();

    top->addToCurrentPage(area);

    parseChildren(node, area->getClientArea(), top);

    area->updateScrollbars(vVisibility, hVisibility, vPageStep, hPageStep, vLineStep, hLineStep, marginLeft, marginBottom);
    area->setupResizeHandler();

    return true;
}

bool XmlParser::parseRepeat(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    UnicodeString name(node.attribute("name").value());

    if (repeatContexts_.count(name) == 0) {
        LOG_ERROR << "Trying to access unknown repeat context " << name << std::endl;
        return false;
    }

    const RepeatContext& context = repeatContexts_[name];

    int xIncrease = node.attribute("xincrease").as_int();
    int yIncrease = node.attribute("yincrease").as_int();
    unsigned int xLimit = node.attribute("xlimit").as_uint();
    unsigned int yLimit = node.attribute("ylimit").as_uint();

    for (unsigned int index = 0; index < context.repeatCount_; ++index) {
        insertRepeatNodes(node.begin(), node.end(), node.parent(), context, index,
                xIncrease, yIncrease, xLimit, yLimit);
    }

    return true;
}

void XmlParser::insertRepeatNodes(pugi::xml_node::iterator begin, pugi::xml_node::iterator end, pugi::xml_node dst,
            const RepeatContext& context, unsigned int index,
            int xIncrease, int yIncrease, unsigned int xLimit, unsigned int yLimit) {
    for (pugi::xml_node::iterator iter = begin; iter != end; ++iter) {
        pugi::xml_node newNode = dst.insert_copy_after(*iter, dst.last_child());

        checkRepeatIf(newNode, index, xLimit, yLimit);

        replaceRepeatKeywords(newNode, context, index,
                xIncrease, yIncrease, xLimit, yLimit);
    }
}

void XmlParser::checkRepeatIf(pugi::xml_node& node, unsigned int index, unsigned int xLimit, unsigned int yLimit) {
    unsigned int xIndex = xLimit > 0 ? index % xLimit : index;
    unsigned int yIndex = yLimit > 0 ? index % yLimit : index;

    bool removeNode = false;
    if (strcmp(node.name(), "repeatif") == 0) {
        pugi::xml_node::attribute_iterator attrIter = node.attributes_begin();
        pugi::xml_node::attribute_iterator attrEnd = node.attributes_end();

        for (; attrIter != attrEnd; ++attrIter) {
            if (strcmp(attrIter->name(), "xindex") == 0) {
                if (attrIter->as_uint() != xIndex) {
                    removeNode = true;
                    break;
                }
            } else if (strcmp(attrIter->name(), "yindex") == 0) {
                if (attrIter->as_uint() != yIndex) {
                    removeNode = true;
                    break;
                }
            }
        }

        if (!removeNode) {
            // move children to parent node
            pugi::xml_node childIter = node.last_child();
            while (childIter) {
                pugi::xml_node newChild = node.parent().insert_copy_after(childIter, node);
                checkRepeatIf(newChild, index, xLimit, yLimit);
                childIter = childIter.previous_sibling();
            }
        }

        // remove repeatif node
        node.parent().remove_child(node);
    } else {
        pugi::xml_node childIter = node.first_child();
        while (childIter) {
            checkRepeatIf(childIter, index, xLimit, yLimit);
            childIter = childIter.next_sibling();
        }
    }
}

void XmlParser::replaceRepeatKeywords(pugi::xml_node& node, const RepeatContext& context, unsigned int index,
            int xIncrease, int yIncrease, unsigned int xLimit, unsigned int yLimit) {

    static std::string attrNameX("x");
    static std::string attrNameY("y");

    pugi::xml_node::attribute_iterator attrIter = node.attributes_begin();
    pugi::xml_node::attribute_iterator attrEnd = node.attributes_end();

    for (; attrIter != attrEnd; ++attrIter) {
        bool contextHit = false;

        std::map<UnicodeString, std::vector<UnicodeString> >::const_iterator contextIter = context.keywordReplacments_.begin();
        std::map<UnicodeString, std::vector<UnicodeString> >::const_iterator contextEnd = context.keywordReplacments_.end();

        for (; contextIter != contextEnd; ++contextIter) {
            if (contextIter->first == attrIter->value()) {
                contextHit = true;

                attrIter->set_value(StringConverter::toUtf8String(contextIter->second[index]).c_str());
                break;
            }
        }

        if (!contextHit) {
            // increase and y values, if found
            if (attrNameX == attrIter->name()) {
                int baseX = attrIter->as_int();
                unsigned int xIndex = xLimit > 0 ? index % xLimit : index;
                int curXIncrease = xIncrease * xIndex;
                int curX = baseX + curXIncrease;
                attrIter->set_value(curX);
            } else if (attrNameY == attrIter->name()) {
                int baseY = attrIter->as_int();
                unsigned int yIndex = yLimit > 0 ? index % yLimit : index;
                int curYIncrease = yIncrease * yIndex;
                int curY = baseY + curYIncrease;
                attrIter->set_value(curY);
            }
        }
    }

    // also apply keyword replacements to child nodes
    pugi::xml_node childIter = node.first_child();
    while (childIter) {
        replaceRepeatKeywords(childIter, context, index,
                xIncrease, yIncrease, xLimit, yLimit);
        childIter = childIter.next_sibling();
    }
}

bool XmlParser::parseWorldView(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    ui::components::WorldView* worldView = new ui::components::WorldView(parent, bounds);

    parseId(node, worldView);

    top->addToCurrentPage(worldView);

    return true;
}

bool XmlParser::parsePaperdoll(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    ui::components::PaperdollView* pdView = new ui::components::PaperdollView(parent, bounds);

    parseId(node, pdView);

    top->addToCurrentPage(pdView);

    return true;
}

bool XmlParser::parseContainer(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    ui::components::ContainerView* contView = new ui::components::ContainerView(parent, bounds);

    parseId(node, contView);

    top->addToCurrentPage(contView);

    return true;
}

bool XmlParser::parseCheckbox(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    unsigned int switchId = node.attribute("switchid").as_uint();
    bool isChecked = node.attribute("checked").as_bool();

    components::UoCheckbox* cb = new components::UoCheckbox(parent);
    cb->switchId_ = switchId;

    parseId(node, cb);

    pugi::xml_node uncheckedNode = node.child("unchecked");
    pugi::xml_node uncheckedMoNode = node.child("unchecked-mouseover");
    pugi::xml_node checkedNode = node.child("checked");
    pugi::xml_node checkedMoNode = node.child("checked-mouseover");

    if (checkedNode && uncheckedNode) {
        parseMultiTextureImage(uncheckedNode, cb, components::UoCheckbox::TEX_INDEX_UNCHECKED);
        parseMultiTextureImage(checkedNode, cb, components::UoCheckbox::TEX_INDEX_CHECKED);
    } else {
        LOG_ERROR << "Checkbox needs to have checked and unchecked image node" << std::endl;
        return false;
    }

    if (uncheckedMoNode) {
        parseMultiTextureImage(uncheckedMoNode, cb, components::UoCheckbox::TEX_INDEX_UNCHECKED_MOUSEOVER);
    }
    if (checkedMoNode) {
        parseMultiTextureImage(checkedMoNode, cb, components::UoCheckbox::TEX_INDEX_CHECKED_MOUSEOVER);
    }

    if (bounds.get_width() == 0 || bounds.get_height() == 0) {
        cb->setAutoResize(true);
        bounds.set_width(1);
        bounds.set_height(1);
    }

    cb->setChecked(isChecked);

    cb->set_geometry(bounds);

    top->addToCurrentPage(cb);
    return true;
}

bool XmlParser::parseSysLogLabel(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    components::SysLogLabel* sysLogLabel = new components::SysLogLabel(top);
    sysLogLabel->setMaxGeometry(bounds);
    parseId(node, sysLogLabel);
    top->addToCurrentPage(sysLogLabel);

    return true;
}

bool XmlParser::parseTBackground(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    components::TBackground* bg = new components::TBackground(parent);
    parseId(node, bg);
    bg->set_geometry(bounds);

    top->addToCurrentPage(bg);
    return true;
}

bool XmlParser::parseWarModeButton(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);

    components::WarModeButton* button = new components::WarModeButton(parent);
    parseId(node, button);

    pugi::xml_node normalNode = node.child("normal");
    pugi::xml_node mouseOverNode = node.child("mouseover");
    pugi::xml_node mouseDownNode = node.child("mousedown");

    if (normalNode) {
        parseMultiTextureImage(normalNode, button, components::WarModeButton::TEX_INDEX_UP);
    } else {
        LOG_ERROR << "Normal image for warmode button not defined" << std::endl;
        return false;
    }

    if (mouseOverNode) {
        parseMultiTextureImage(mouseOverNode, button, components::WarModeButton::TEX_INDEX_MOUSEOVER);
    }
    if (mouseDownNode) {
        parseMultiTextureImage(mouseDownNode, button, components::WarModeButton::TEX_INDEX_DOWN);
    }


    pugi::xml_node warmodeNormalNode = node.child("warmode-normal");
    pugi::xml_node warmodeMouseOverNode = node.child("warmode-mouseover");
    pugi::xml_node warmodeMouseDownNode = node.child("warmode-mousedown");

    if (warmodeNormalNode) {
        parseMultiTextureImage(warmodeNormalNode, button, components::WarModeButton::WARMODE_TEX_INDEX_UP);
    }
    if (warmodeMouseOverNode) {
        parseMultiTextureImage(warmodeMouseOverNode, button, components::WarModeButton::WARMODE_TEX_INDEX_MOUSEOVER);
    }
    if (warmodeMouseDownNode) {
        parseMultiTextureImage(warmodeMouseDownNode, button, components::WarModeButton::WARMODE_TEX_INDEX_DOWN);
    }

    if (bounds.get_width() == 0 || bounds.get_height() == 0) {
        button->setAutoResize(true);
        bounds.set_width(1);
        bounds.set_height(1);
    }

    button->updateTexture();

    button->set_geometry(bounds);

    top->addToCurrentPage(button);
    return true;
}

bool XmlParser::parseTClickLabel(pugi::xml_node& node, CL_GUIComponent* parent, GumpMenu* top) {
    CL_Rect bounds = getBoundsFromNode(node);
    std::string align = node.attribute("align").value();
    std::string text = node.attribute("text").value();

    unsigned int buttonId = node.attribute("buttonid").as_uint();
    unsigned int pageId = node.attribute("page").as_uint();
    UnicodeString action = StringConverter::fromUtf8(node.attribute("action").value());
    UnicodeString param = StringConverter::fromUtf8(node.attribute("param").value());
    UnicodeString param2 = StringConverter::fromUtf8(node.attribute("param2").value());
    UnicodeString param3 = StringConverter::fromUtf8(node.attribute("param3").value());
    UnicodeString param4 = StringConverter::fromUtf8(node.attribute("param4").value());
    UnicodeString param5 = StringConverter::fromUtf8(node.attribute("param5").value());

    components::ClickLabel* label = new components::ClickLabel(parent);
    parseId(node, label);

    if (action.length() > 0) {
        label->setLocalButton(action);
        label->setParameter(param, 0);
        label->setParameter(param2, 1);
        label->setParameter(param3, 2);
        label->setParameter(param4, 3);
        label->setParameter(param5, 4);
    } else if (!node.attribute("buttonid").empty()) {
        label->setServerButton(buttonId);
    } else if (!node.attribute("page").empty()) {
        label->setPageButton(pageId);
    } else {
        LOG_WARN << "ClickLabel without action, id or page" << std::endl;
        return false;
    }

    if (align.length() == 0 || align == "left") {
        label->set_alignment(CL_Label::align_left);
    } else if (align == "right") {
        label->set_alignment(CL_Label::align_right);
    } else if (align == "center") {
        label->set_alignment(CL_Label::align_center);
    } else if (align == "justify") {
        label->set_alignment(CL_Label::align_justify);
    } else {
        LOG_WARN << "Unknown clicklabellabel align: " << align << std::endl;
        return false;
    }

    label->set_text(text);
    label->set_geometry(bounds);

    top->addToCurrentPage(label);
    return true;
}

}
}
