--[[
ͨ��ǰ������ lua �ļ��Ľ��ܣ����Ŵ�Ҷ� AI �ı�д�Ѿ����˻�������ʶ��

��һ�ĵ������н����� smart-ai.lua �е������֡���Ӧ���󡱵���غ�������� 
����һ�ĵ����ܵ����ݿ�ʼ����ҪΪ AI ��д�����ߵĴ����ˡ�
Ϊ���� AI ������ȷ�ľ��ߣ������ AI ���㹻����Ϣ������һ����������ͨ�� data �������ݡ�
���� 11-Fundamentals.lua ���Ѿ������ἰ����������ؽ���һ�� data ��ص�һЩ���ݡ�

++ data ��ʲô��
data ��һ�� QVariant ������һ���������ʹ�������Դ����������͵���Ϣ��
������һ��������һ���ƣ�һ��ʹ���ƵĽṹ�壨CardUseStruct����һ����ң�ServerPlayer*�����ȵȡ�

++ ��ι��� data��
�� C++ �ͨ�� QVariant data = QVariant::fromValue(object) ���� data��
���� object �����������ᵽ���κ�һ�ֶ���
�� Lua �ͨ������Ĵ��������� data��]]
local data = sgs.QVariant() -- ����һ���յ� QVariant ����
data:setValue(object) -- Ϊ QVariant ��������ֵ���൱������� QVariant::fromValue(object)
--[[
++ ����� AI ��� data �õ���Ӧ��ֵ��
�����������͵Ĳ�ͬ����Ҫ�ò�ͬ�ĺ������б����¡���ߵ���ת����õ��Ķ������ͣ��ұ�����Ӧ��ת��������
��Щ����ʵ���Ͽ����� swig/qvariant.i �����ҵ���ע�����еĽṹ�嶼������ָ�����ʽ������ Lua �еġ�
����ȷ�е�˵��һ������ı�����ϸ�ڿɲμ� SWIG ���ĵ���
number����ֵ���ͣ�	data:toInt()							ע�⣺Lua ����û�� int ���ͣ�ֻ�� number ����
string���ַ�����	data:toString()
string ��ɵı�		data:toStringList()						ע�⣺toStringList() �õ����Ǳ�table���������б�QList��
bool������ֵ��		data:toBool()
DamageStruct*		data:toDamage(), data:toDamageStar()	ע�⣺����û��ʵ���ϵ�����
CardEffectStruct*	data:toCardEffect()
SlashEffectStruct*	data:toSlashEffect()
CardUseStruct*		data:toCardUse()
CardMoveStruct*		data:toCardMove()
Card*				data:toCard()
ServerPlayer*		data:toPlayer()
DyingStruct*		data:toDying()
RecoverStruct*		data:toRecover()
JudgeStruct*		data:toJudge()
PindianStruct*		data:toPindian()

���������ᵽ����Щ�ṹ��ľ��庬�弰�����ݳ�Ա���Ѿ��������ĵ��ķ�Χ���μ� src/server/structs.h

���潫���� smart-ai.lua �е������ֵ�һЩ��������صı�
% skill_name��������Ĵ���������г䵱��������ʾ��������
% self������������ĺ���ԭ���еĵ�һ����������ʾ����� SmartAI ����

! SmartAI:askForSuit()
�ú���������Ӧ��ѡ��ɫ��������
% ����ֵ��Card::Suit ֮һ���� sgs.Card_Spade, sgs.Card_Heart, sgs.Card_Club, sgs.Card_Diamond ֮һ
% ��صı�����
% Ĭ����Ϊ�������� 2:2:2:3 �ı������ѡ����ҡ����ҡ��ݻ�����Ƭ

! SmartAI:askForSkillInvoke(skill_name, data)����Ӧ Room::askForSkillInvoke �ĺ�����
�ú���������Ӧ���Ƿ񷢶����� skill_name��������
% ����ֵ������ֵ����ʾ�Ƿ񷢶��ü���
% ��صı�sgs.ai_skill_invoke
% Ĭ����Ϊ��������ܵķ���Ƶ�ʣ�Frequency��Ϊ sgs.Skill_Frequent���򷢶������򲻷���

* sgs.ai_skill_invoke��
% Ԫ�����ƣ�������
% Ԫ�أ�����ֵ����
%% ����ֵһ��Ϊ true����ʾ�ü��ܲ���ʲô���������
%% ������ԭ��Ϊ��function(self, data)
%% ����ֵ���� SmartAI.askForSkillInvoke ���еķ���ֵ�����ԣ�������ͬ

% ���� 1��mountain-ai.lua �� 168 ��]]
sgs.ai_skill_invoke.tuntian = true -- �������Ƿ���
--[[% ���� 2��thicket-ai.lua �� 55 �� 58 �У��μ� 11-Fundamentals.lua

! SmartAI:askForChoice(skill_name, choices)����Ӧ Room::askForChoice �ĺ���
�ú���������Ӧ����ѡ�񡱵�����
���û������б���Ϊһ���Լ�����Ϊ���⣬��Ӧ��ÿһ��ѡ����һ����ť�ĶԻ���
% choices: ���������п���ѡ��
% ����ֵ���ַ������� choices �е�һ���ʾ������ѡ��
% ��صı�sgs.ai_skill_choice
% Ĭ����Ϊ��ͨ������ Skill::getDefaultChoice ��ü��ܵ�Ĭ��ѡ��Ĭ��ѡ��ȱʡΪ "no"����
���Ĭ��ѡ���� choices �У��򷵻�Ĭ��ѡ�񡣷���������� choices �е�һ��Ԫ�ء�

* sgs.ai_skill_choice��
% Ԫ�����ƣ�������
% Ԫ�أ��ַ���ֵ����
%% �ַ���ֵ���������ۺ�������¶�����ͬһ��������ѡ��
%% ������ԭ��Ϊ function(self, choices)
%% choices, ����ֵ: �� SmartAI.askForChoice ������ͬ
% ���� 1��god-ai.lua �� 460 ��]]
sgs.ai_skill_choice.jilve="zhiheng" -- ����ѡ�����ƺ⻹����ɱʱ������ѡ���ƺ⡣
--% ���� 2��yitian-ai.lua �� 484 �� 487 ��
sgs.ai_skill_choice.yisheask=function(self,choices)
	assert(sgs.yisheasksource) -- �����Һ�����仰
	if self:isFriend(sgs.yisheasksource) then return "allow" else return "disallow" end
	-- �������Ҫ�Ƶ����������ѷ�����������󣬷���ܾ�����
end
--[[
! SmartAI:askForDiscard(reason, discard_num, optional, include_equip)��
��Ӧ Room::askForDiscard �� Room::askForExchange �ĺ���
�ú���������Ӧ �������� X ���ơ� �� ����ѡ�� X ���ƽ��н����� ������
% reason���ַ��������Ƶ�ԭ��һ��Ϊ����������������������ƽ׶ε����ƣ���Ϊ��gamerule��
% discard_num����ֵ���������Ƶ�����
% optional������ֵ���Ƿ����ѡ������
% include_equip������ֵ���Ƿ�������װ��
% ����ֵ������������Ҫ�����Ƶ� ID
% ��صı�sgs.ai_skill_discard
% Ĭ����Ϊ��������ѡ�����ƣ����������򰴱���ֵ��С�����������ơ�
���������װ������ sgs.lose_equip_skill �еļ��ܣ���������װ����

* sgs.ai_skill_discard��
% Ԫ�����ƣ�����ԭ�򣨼� reason��
% Ԫ�أ�������ԭ��Ϊ function(self, discard_num, optional, include_equip)
%% discard_num, optional, include_equip, ����ֵ���� SmartAI.askForDiscard ������ͬ
������ nil����ִ��Ĭ����Ϊ��
% ���ӣ�standard-ai.lua �� 82 �� 108 �й��ڸ��ҵĴ��롣]]
sgs.ai_skill_discard.ganglie = function(self, discard_num, optional, include_equip)
	if self.player:getHp() > self.player:getHandcardNum() then return {} end
	-- ������ֵ���������࣬�����ơ��˲����д���ȶ��

	if self.player:getHandcardNum() == 3 then -- ������Ϊ 3 ʱ���ٽ����Σ�
		local to_discard = {} -- ��ʼ�� to_discard Ϊ�ձ�
		-- ��һ�䲻��ʡ�ԣ����� table.insert(to_discard, ...) �ᱨ��
		local cards = self.player:getHandcards() -- �����������
		local index = 0
		local all_peaches = 0
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") then
				all_peaches = all_peaches + 1 -- ����������С��ҡ���������
			end
		end
		if all_peaches >= 2 then return {} end -- �������� 2 �š��ҡ��������ơ�

		for _, card in sgs.qlist(cards) do
			if not card:inherits("Peach") then
				table.insert(to_discard, card:getEffectiveId())
				-- �Ѳ��ǡ��ҡ����Ƶ� ID ���뵽�����б�֮��
				index = index + 1
				if index == 2 then break end -- �������б����Ѿ��������Ƶ� ID������ֹѭ��
				-- �˴�ȥ���ֲ����� index ������ #to_discard ��ʹ�����Ϊ���
			end
		end
		return to_discard -- ���������б�
	end

	if self.player:getHandcardNum() < 2 then return {} end -- ������������ 2 �ţ����޷����ơ�
end -- �������������Ĭ����Ϊ�����ƽ׶εĲ��ԣ����ơ�
--[[
�������һ�䣬�������ע�Ϳ��Կ������� AI �Ĳ��Ի��кܶ಻���Ƶĵط���
����Ҳ���Ƚ��࣬�������Ժ���Ҫ��Ŭ���Ľ��ġ�

! SmartAI:askForNullification(trick_name, from, to, positive)��
��Ӧ Room::askForNullification �ĺ������ú���������Ӧ���Ƿ�ʹ�á���и�ɻ�����������
% trick_name��Card* ���ͣ���ʾ�Ժ��Ž�����ʹ����и�ɻ�
������������ʹ������Ϊ���ַ������ͣ������Ժ��޸ģ�
% from��ServerPlayer*��trick_name ��ʹ���ߣ����ǡ���и�ɻ�����ʹ���ߣ�
% to: ServerPlayer*��trick_name ��ʹ�ö���
% positive��Ϊ true ʱ��������и�ɻ���ʹ trick_name ʧЧ�����򱾡���и�ɻ���ʹ trick_name ��Ч
% ����ֵ��Card*������ʹ�õġ���и�ɻ��������Ϊ nil����ʾ��ʹ�á���и�ɻ���
% ��صı���
% Ĭ����Ϊ���ϸ��ӣ��򵥵�˵���Ǹ��ݽ����Ƿ�Լ������������Ƿ�ʹ�á�
����Ȥ�Ŀɲμ� smart-ai.lua �е�Դ���롣

! SmartAI:askForCardChosen(who, flags, reason)����Ӧ Room::askForCardChosen �ĺ���
�ú���������Ӧ����Ӹ���������ѡ��һ�š�������
���û������б���Ϊ����ʹ�á�˳��ǣ�򡿵�ʱ����ֵĶԻ���
% who��ServerPlayer*���Ӻ��˵�����ѡ��
% flags���ַ�����"h", "e", "j" ��������ϣ��μ� 12-SmartAI.lua
% reason���ַ����������ԭ�򣬿����Ǽ����������ǿ��ƵĶ������������� "snatch"��
% ����ֵ����ֵ��ѡ���ʵ�忨�� ID��
% ��صı�sgs.ai_skill_cardchosen
% Ĭ����Ϊ����ʹ�á����Ӳ��š���ʱ��ѡ���ƵĲ��ԣ���Ϊ���ӣ�����Ȥ�Ŀɲμ� smart-ai.lua �е�Դ���롣

? sgs.ai_skill_cardchosen��
% Ԫ�����ƣ�reason���������еĶ̺� "-" Ҫ���»��� "_" ȡ����
% Ԫ�أ�������ԭ��Ϊ cardchosen(self, who)
%% who, ����ֵ���� SmartAI.askForCardChosen ������ͬ
����ֵΪ nil ʱ��ִ��Ĭ����Ϊ��
% ���ӣ�ֻ��һ������ mountain-ai.lua �� 70 �� 74 �С�������Ϊ���󲿷������Ĭ����Ϊ�Ѿ�������Ҫ��]]
sgs.ai_skill_cardchosen.qiaobian = function(self, who, flags)
	if flags == "ej" then
		return card_for_qiaobian(self, who, "card")
		-- ���� mountain-ai.lua �� 1 �п�ʼ����ĸ��������õ������
	end
end
--[[
! SmartAI:askForCard(pattern, prompt, data)����Ӧ Room::askForCard �ĺ���
�ú���������Ӧ������һ���ơ�������
���û������б���Ϊһ����ʾ�򣬿��������� prompt �뷭���ļ�������