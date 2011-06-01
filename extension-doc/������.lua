--�������3��TriggerSkill ������

--�󲿷ּ��ܶ��С���xxʱִ��Ч�����������������κη������ļ���Ҳ�������ô����ķ�ʽ��ʵ�֡�
--Ҳ����Ϊ��ˣ����������෱�࣬Ҳ�кܶ಻ͬ�Ķ��巽ʽ��
--�����ģ�Ҳ�����ܵģ�������������Lua����ʹ��sgs.CreateViewAsSkill�������÷���������lua\sgs_ex.lua���ҵ���

--CreateViewAsSkill��Ҫ���²�����

--name, frequency����ѡ��, events, on_trigger, can_trigger(��ѡ)

--����nameΪ���������ִ�

--frequency�Ǽ��ܵķ���Ƶ�ʣ�Ĭ��Ϊ������������Ҳ����˵�����㷢������ʱѯ�ʷ�������������Ҫ�����ｫfrequency��Ϊcompulsoryǿ�Ʒ�����

--eventsΪ����ʱ���������ʱ���б��һ��ṩ�ο��ĵ���events������lua tableҲ������һ��������events enum(�����Ͼ���int)

--on_triggerΪ����Ч�����������ھ��ж������ʱ���ļ���(��ӹ��)����Ҫʹ��������䣬��ʱ��ִ��Ч����

--can_triggerΪ�������������жϡ��������㷢��������������˺���Ӧ�÷���true���޶���ʱ��Ĭ��Ϊ�������㴥��������

--�����ǲܲټ��۵�ʵ�֣�

jianxiong=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_Frequent,
	
	name      = "jianxiong",
	
	events={sgs.Damaged}, --����events=sgs.Damaged
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local card = data:toDamage().card
		if not room:obtainable(card,player) then return end
		if room:askForSkillInvoke(player,"jianxiong") then
			room:playSkillEffect("jianxiong")
			player:obtainCard(card)
		end
	end
}
--��on_trigger�����У��������Ȼ�ȡ��room����

--����Ӱ��������Ϸ��Ч�������Ǳ�����Ҫ��ȡroom���󡣴��������£�room�����Ǳ����ȡ�ġ�

--on_trigger������data������һ��QVariant�����ݲ�ͬ���¼�������Ҫ�ò�ͬ�ķ����õ���ԭ�����������͡�
--����Damaged�¼������ܵ����˺�����data�����������DamageStruct������ʹ��toDamage()�õ�DamageStruct��

--ѯ�ʼ��ܷ���ʱ����Ҫʹ��room�����askForSkillInvoke������
--playSkillEffect��������Բ��ż��ܵķ���Ч���������ǶԼ��ܷ���Ч������û��Ӱ�죩

--player:obtainCard(card) ����player�õ�����˺���card��

--�ڡ�ĳ���׶οɴ������ļ��ܣ����ߡ�����ʱ��Ϊxx�������ļ��ܣ�����ʹ��PhaseChange�¼�������������event��������жϽ��д������ơ�

--�����ڸ�����ʱ�������Ĵ�������������Ҫ��on_trigger��ʹ��������䡣

--������Ԭ����ӹ�������ܵ�ʵ�֣�

yongsi=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_Compulsory, --������
	
	name      = "yongsi",
	
	events={sgs.DrawNCards,sgs.PhaseChange}, --��������ʱ��
	
	on_trigger=function(self,event,player,data)
	
		local room=player:getRoom()
		
		local getKingdoms=function() --�����ں����ж��庯��
			local kingdoms={}
			local kingdom_number=0
			local players=room:getAlivePlayers()
			for _,aplayer in sgs.qlist(players) do
				if not kingdoms[aplayer:getKingdom()] then
					kingdoms[aplayer:getKingdom()]=true
					kingdom_number=kingdom_number+1
				end
			end
			return kingdom_number
		end
		
		if event==sgs.DrawNCards then 
			room:playSkillEffect("yongsi")
			data:setValue(data:toInt()+getKingdoms()) --DrawNCards�¼���data��һ��int���͵�QVariant���ı��QVariant�����ı�������
		elseif (event==sgs.PhaseChange) and (player:getPhase()==sgs.Player_Discard) then
			--�������ƽ׶�ʱ����ִ��ӹ�����ƣ�Ȼ����ִ�г�������
			room:output("aaaaaa")
			local x = getKingdoms()
			local e = player:getEquips():length()+player:getHandcardNum()
			if e>x then room:askForDiscard(player,"yongsi",x,false,true) -- �����������Ϊ���Ƿ�ǿ�ơ��Լ����Ƿ����װ����
			else 
				player:throwAllHandCards()
				player:throwAllEquips()
			end
		end
	end
}