--�������1��ViewAsSkill

--��̫��������ɱ�У��������ܿɷ�Ϊ���ࣺ����������Ϊ����ϵͳ����

--��������������ʵ�֡���ĳ���׶Ρ����㷢������ʱ��ִ��ĳ��Ч������������ѡ�������ļ��ܡ�
--������Ҳ���������ı���Ϸ�¼��������ǵ����Ĳ���Ч�������磬�������ƽ׶β�ִ��xx�����ļ���Ҳ�����ô�����ʵ�֡�

--��Ϊ����������ʵ�֡��ɽ�ĳ����Ϊĳ�ƴ���������ļ��ܡ�
--��Ϊ���Ķ������AI��������Ч�ġ�Ϊ����AIʹ����Ϊ�������������Ҫ��AI������дһ�鼼�ܵĶ��塣
--��Ϊ����ϵ����һ��ȱ�������ںܶ�AI���ӹ���ĸ���ԭ��

--���ǿ��Խ�����������Ϊ��������������ܡ��ڴ������ж���һ����Ϊ������������Ϊ���ڴ������������¼��з�����
--������ĸ��Ӽ��ܶ���Ҫͨ����ϴ���������Ϊ����ʵ�֡�

--ϵͳ�����κ��޷����������ּ���ʵ�ֻ�������������ļ��ܡ����磬��ɱ���������������������ļ��ܡ�
--���༼��ͨ���ı�����Ϸ�������򣬾��С�����޷�����ĳ��ѡ�񡱡�����ҿ�������ĳ��ѡ�񡱵�Ч����
--����ʹ���ˡ��������ʱ��������δ����ʱ��������Ϊֹ�������ļ��ܶ��ǿ�ֱ�Ӱ���������������ʵ�ֵġ�

--��ˣ����������ص�ļ��ܣ���Ҫ��ϵͳ��ʵ�֣�����ʱ�޷�DIY�ġ�
--���ڡ���ҿ�������ĳ��ѡ�������ļ��ܣ���������������ʹ�á�����Խ�һ��ɱ��Ϊһ��ɱ�����������ʵ�֡�
--������޷�����ĳ��ѡ�������ļ��ܣ��뾡�����⡣ע�⣬��Ч����ļ�������ȫ���Եġ�

--
--��Ƭ��ʼ



--���Ƚ�����Ϊ����
--��Ϊ����Lua�еĴ���ʹ����sgs.CreateViewAsSkill�������÷���������lua\sgs_ex.lua���ҵ���
--��Ϊ���ڴ���ʱ����Ҫ���·���|�����Ķ��壺

--name,	n, view_filter, view_as�Լ���ѡ��enabled_at_play��enabled_at_response

--nameΪһ���ַ������������ơ�

--nΪÿ�η��������������������ֵ��������������Ϊ998��ֻҪ998�����������DIY�õ���n���ܶ�Ϊ1��2.


--view_filterΪĳ�ſ��Ƿ�ɱ�ѡ���������������ܡ���������ʱ�������������ơ�װ�����б�������ִ��view_filter������������true���ƿ��Ա�ѡ���������ܷ�����

--����Ϊ������һ�Ųݻ��ơ���view_filter������

n=1,

view_filter = function(self, to_select, selected)
	local condition=(to_select:getSuit()==sgs.Card_Club)
	return condition
end,

--to_selectΪ���ڱ������ƵĶ������á�������Ļ�ɫΪ�ݻ�Club���򷵻��棨�ɱ�ѡ�񣩡����򷵻ؼ٣����ɱ�ѡ�񣩡�

--����Ϊ����������ͬ��ɫ���ơ���view_filter������

n=2,

view_filter = function(self, to_select, selected)
	if #selected<1 then return not to_select:isEquipped() end
	local condition=(to_select:getSuit()==selected[1]:getSuit())
	return condition and not to_select:isEquipped()
end,

--selectedΪһ��lua table�������Ѿ�ѡ�е������Ƶ����á�������ѡ�е�����С��1����ô�κ��ƶ����Ա�ѡ�����ѡ�е�������С��1����ôֻ����Щ���ѱ�ѡ�еĵ�һ���ƻ�ɫ��ͬ���Ʋ��ܱ�ѡ�С�


--view_as�����������յõ��ģ�������Ϊ��������ƵĶ���������ƿ�������Ϸ�ƣ�Ҳ�����Ǽ����ơ�������DIY���ܵ�Ч������ĳ����Ϸ�Ƶ�Ч������ô����Ҫ�Ѹ�Ч�����嵽һ�������Ƶ��У�Ȼ����view_as�����еõ�������һ���㶨��ļ����ơ�

--����Ϊ�����ɽ赶ɱ��ʹ�á���view_as�������֣�

n=1,

view_as = function(self, cards)

	local invalid_condition=(#cards<1)
	if invalid_condition then return nil end
	
	local suit,number
	
	for _,card in ipairs(cards) do
		if suit and (suit~=card:getSuit()) then suit=sgs.Card_NoSuit else suit=card:getSuit() end
		if number and (number~=card:getNumber()) then number=-1 else number=card:getNumber() end
	end
	
	local view_as_card= sgs.Sanguosha:cloneCard("collateral", suit, number)
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	
	view_as_card:setSkillName(self:objectName())
	
	return view_as_card
end,

--��ѡ�е�����ĿС��1���򷵻ؿն����ʾû��Ч���ƿ��Բ�����
--���򣬼�¼�ƵĻ�ɫ����ֵ������ѡ�е��Ƶ��л�ɫ����ֵ�в�ͬ����Ϊ�޻�ɫ�������֡�

--��¼֮�����ɽ赶ɱ���ơ����ƵĻ�ɫ������֮ǰ��¼����ͬ��
--��ʹ�õ����Ƽ������ɵĽ赶ɱ�˵������б��С���һ��ͨ���Ǳ���ģ���Ϊ���ܴ��ڶ��ƽ��л����ļ��ܣ������һ�����ۣ�
--�����ܵ����ּ�self:objectName����ÿ��ļ��������ԡ�

--��Ҫ������Ϸ��ʱ���Ķ�collateral������ּ��ɡ���Ҫ���ݲ�ͬ���ࡢ�������Ƶõ���ͬ��Ϸ��ʱ���Ķ�invalid_condition������cloneCard���ɡ�

--�����ܵ�Ч�����ܼ򵥵�����Ϊ����Ϊ������xx�ơ�ʱ������Ҫʹ�ü����ƶ��弼�ܵ�Ч����Ҳ����˵������ļ���ת��Ϊ������Խ����xx����Ϊxx�ƴ��������xx�Ƶ�Ч��Ϊblahblah����Ȼ���ڼ����ƵĶ�����ʵ�ּ��ܵ�Ч����

--Ҳ����˵�����ܵ�Ч���á������ơ�ʵ�֣����ܵķ���Լ���á���Ϊ����ʵ�֣����ܵķ���ʱ�������á���������ʵ�֡�

--������������似�ܵ�view_as������

view_as = function(self, cards)

	local invalid_condition=(#cards<1)
	if invalid_condition then return nil end
	
	local view_as_card=sgs.CreateSkillCard( lijianCard )
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	
	return view_as_card
end,

--����lijianCard�Ķ���Ӧ�ñ�������ͬһ��module�ļ����С��ҽ��������ĵ��н��⼼���ƵĶ��塣