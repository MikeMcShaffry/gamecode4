-- Utility.lua
-- A simple utility game

-- A "better" random seed.  This was taken from http://lua-users.org/wiki/MathLibraryTutorial
math.randomseed(tonumber(tostring(os.time()):reverse():sub(1,6)))

-----------------------------------------------------------------------------------------------------------------------
-- Actor
-----------------------------------------------------------------------------------------------------------------------
Actor = class(nil,
{
	_name = "<unnamed>",
	_hp = 100,
	_mp = 3,
	
	_minDamage = nil,  -- must be overridden
	_maxDamage = nil,  -- must be overridden
	
	_minHeal = 20,
	_maxHeal = 45,
});

function Actor:DoAction(action, opponent)
	if (action == "attack") then
		local dead = self:_Attack(opponent);
		return dead;
	elseif (action == "run") then
		local success = self:_Run();
		return success;
	elseif (action == "heal") then
		self:_Heal();
		return false;
	else
		print("Unknown action: " .. action);
		return false;
	end
end

function Actor:PrintStatus()
	print(self._name .. "; hp = " .. self._hp .. "; mp = " .. self._mp);
end

function Actor:Hit(damage)
	self._hp = self._hp - damage;
	if (self._hp < 0) then
		print(self._name .. " is dead!");
		return true;
	end
	return false;
end

function Actor:GetHp()
	return self._hp;
end

function Actor:GetMp()
	return self._mp;
end

function Actor:GetMinDamage()
	return self._minDamage;
end

function Actor:GetMaxDamage()
	return self._maxDamage;
end

function Actor:GetMinHeal()
	return self._minHeal;
end

function Actor:GetMaxHeal()
	return self._maxHeal;
end

function Actor:_Attack(opponent)
	assert(self._minDamage ~= nil and self._maxDamage ~= nil);
	
	local damage = math.random(self._minDamage, self._maxDamage);
	print(self._name .. " struck for " .. damage .. " damage");
	return opponent:Hit(damage);
end

function Actor:_Run()
	if (math.random() > 0.5) then
		print(self._name .. " ran away!");
		return true;
	else
		print(self._name .. " failed to run away!");
		return false;
	end		
end

function Actor:_Heal()
	if (self._mp > 0) then
		local healAmount = math.random(self._minHeal, self._maxHeal);
		print(self._name .. " healed " .. healAmount .. " hp.");
		self._hp = self._hp + healAmount;
		self._mp = self._mp - 1;
	else
		print(self._name .. " attempted to heal but had no MP left");
	end
end



-----------------------------------------------------------------------------------------------------------------------
-- Teapot
-----------------------------------------------------------------------------------------------------------------------
Teapot = class(Actor,
{
	_name = "Teapot",
	_minDamage = 10,
	_maxDamage = 20,
	_mp = 5,  -- teapots are more magical
	
	_actions = { "attack", "heal", "run" };
});

function Teapot:Decide(opponent)
	local bestUtility = 0;
	local bestAction = nil;

	for i, action in ipairs(self._actions) do
		-- build the world state
		local tempWorldState = WorldState:Create();
		tempWorldState:Build(self, opponent);
		
		-- apply the action
		tempWorldState:ApplyAction(action, self, opponent);

		-- grab the utility
		local utility = tempWorldState:GetUtility();
		if (utility > bestUtility) then
			bestUtility = utility;
			bestAction = action;
		end
		
		print("\tScoring action '" .. action .. "': utility = " .. utility);
	end
	
	
	print("\tChose " .. bestAction);
	return bestAction;
end


-- This is the world state object used by the teapot
WorldState = class(nil,
{
	opponentHp = nil,
	opponentMp = nil,
	survivalChance = nil,  -- the chance that we won't die next attack, from 0 - 1
});

function WorldState:Build(teapot, opponent)
	self.opponentHp = opponent:GetHp();
	self.opponentMp = opponent:GetMp();
	self.survivalChance = self:_CalculateSurvivalChance(teapot:GetHp(), opponent);
	self.killChance = 1 - self:_CalculateSurvivalChance(opponent:GetHp(), teapot);
end

function WorldState:ApplyAction(action, teapot, opponent)
	if (action == "attack") then
		local averageDamage = ((teapot:GetMaxDamage() - teapot:GetMinDamage()) / 2) + teapot:GetMinDamage();
		self.opponentHp = self.opponentHp - averageDamage;
		self.killChance = 1 - self:_CalculateSurvivalChance(self.opponentHp, teapot);
	elseif (action == "run") then
		self.survivalChance = 0.5;
	elseif (action == "heal") then
		if (teapot:GetMp() > 0) then
			local averageHeal = ((teapot:GetMaxHeal() - teapot:GetMinHeal()) / 2) + teapot:GetMinHeal();
			local newHp = teapot:GetHp() + averageHeal;
			self.survivalChance = self:_CalculateSurvivalChance(newHp, opponent);
		end
	else
		print("Unknown action: " .. action);
		return false;
	end
end

function WorldState:GetUtility()
	local lifeScore = 100 * self.survivalChance;  -- a state in which we're alive grants 100 points, modified by the chance that we might survive
	local attackScore = 100 - self.opponentHp;  -- a state in which the opponent is hurt is always good; teapot will get more aggressive as player is hurt
	attackScore = attackScore + (attackScore * self.killChance);  -- adjust the attack score by up to double if the opponent is within a turn of death
	return lifeScore + attackScore;
end

function WorldState:_CalculateSurvivalChance(defenderHp, attacker)
	if (defenderHp > attacker:GetMaxDamage()) then
		return 1;
	elseif (defenderHp <= attacker:GetMinDamage()) then
		return 0;
	else
		local range = attacker:GetMaxDamage() - attacker:GetMinDamage();
		local percentage = (defenderHp - attacker:GetMinDamage()) / range;
		return percentage;
	end
end


-----------------------------------------------------------------------------------------------------------------------
-- Player
-----------------------------------------------------------------------------------------------------------------------
Player = class(Actor,
{
	_name = "Player",
	_minDamage = 15,
	_maxDamage = 30,
});


-----------------------------------------------------------------------------------------------------------------------
-- Game class
-----------------------------------------------------------------------------------------------------------------------
Game = class(nil,
{
	_player = nil,
	_teapot = nil,
});

function Game:Start()
	self._player = Player:Create();
	self._teapot = Teapot:Create();

	print("A Teapot draws near!  Command?");
	print("1) Attack");
	print("2) Run");
	print("3) Heal");
	print("4) Status");
	print("Q to quit");
end

function Game:Update(action)
	-- player runs action
	local gameOver = self._player:DoAction(action, self._teapot);
	
	-- check for game end
	if (gameOver) then
		self:_Reset();
		return;
	end
	
	-- teapot action
	action = self._teapot:Decide(self._player);
	gameOver = self._teapot:DoAction(action, self._player);
	
	-- check for game end
	if (gameOver) then
		self:_Reset();
		return;
	end
	
	self:ShowStatus();
end

function Game:ShowStatus()
	self._player:PrintStatus();
	self._teapot:PrintStatus();
end

function Game:_Reset()
	print("Game over!\n\n");
	self:Start();
end


-----------------------------------------------------------------------------------------------------------------------
-- Global game object
-----------------------------------------------------------------------------------------------------------------------
g_game = Game:Create();
g_game:Start();


-----------------------------------------------------------------------------------------------------------------------
-- Player commands
-----------------------------------------------------------------------------------------------------------------------
function Attack()
	g_game:Update("attack");
end

function Run()
	g_game:Update("run");
end

function Heal()
	g_game:Update("heal");
end

function Status()
	g_game:ShowStatus();
end

