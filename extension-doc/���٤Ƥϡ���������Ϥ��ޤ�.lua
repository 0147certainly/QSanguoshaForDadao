--��Һ�����hypercross��
--������ļ���ʼ����DIY�ӿڵ��÷���

--���ȣ�����ļ�˵��DIY��Ҫ���ļ�����ṹ��

--DIY����module����ʽ���ڵġ�ÿ��Module����һ��UTF8��ʽ��Lua�ļ���������notepad++�༭�����������¸�ʽ�Ĵ��룺

module("extensions.moligaloo", package.seeall)  -- ����module������moligaloo����ʱ�����ļ�����ͬ��

extension = sgs.Package("moligaloo")            -- ������չ�����󡣱���������Ϊextension��������Ϊ��չ����objectName��Ҳ��ͨ����ʹ�õ���չ����ʶ

shiqian = sgs.General(extension, "shiqian", "qun") -- �����佫���󡣹����佫���Ե���ϸ˵����reference�ĵ���

shentou = sgs.CreateViewAsSkill{ --�������ܣ���������ΪViewAsSkill�� �˶����ڴ����ظ��Դ���������һ���ű������������ɴ��룻���������������ᳫ�ű�����������������Ӧ�ý�����ViewAsSkill�������������ѡ�
	name = "shentou",
	n = 1,
	view_filter = function(self, selected, to_select)
		return to_select:getSuit() == sgs.Card_Club and not to_select:isEquipped()
	end,
	
	view_as = function(self, cards)
		if #cards == 1 then
			local card = cards[1]
			local new_card =sgs.Sanguosha:cloneCard("snatch", card:getSuit(), card:getNumber())
			new_card:addSubcard(card:getId())
			new_card:setSkillName(self:objectName())
			return new_card
		end
	end
}--���ڼ��ܵ�˵�����Ǽ����������������ļ����ص㡣�˴�ʡ�ԡ�

shiqian:addSkill(shentou) --�����佫���ܡ�

--����Խ����ļ�������extensionĿ¼�µ�moligaloo.lua��������Ϸ����ʱ��չ�����Ѿ����������Ϸ��

--Ϊ������DIY��չ������Ҫ����Ƶ��ͼƬ�Լ��������ŵ�ָ��Ŀ¼����һ�㽫�������ĵ���˵����