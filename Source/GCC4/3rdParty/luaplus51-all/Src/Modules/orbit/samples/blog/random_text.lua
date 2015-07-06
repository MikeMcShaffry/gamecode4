
local stuff = {};

stuff[1] = "There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.";

stuff[2] = "I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.";

stuff[3] = "Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.";

stuff[4] = "One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.";

stuff[5] = "Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.";

stuff[6] = "Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. ";

stuff[7] = "This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!";

stuff[8] = "Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.";

stuff[9] = "80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.";

stuff[10] = "Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!";

stuff[11] = "Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.";

stuff[12] = "Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.";

stuff[13] = "Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!";

stuff[14] = "Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.";

stuff[15] = "Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.";

stuff[16] = "Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.";


local function shuffle()
	local swapFrom, swapTo, swapFromContent, swapToContent;
	for i=1,50 do
		swapFrom = math.random(1, #stuff)
		swapTo = math.random(1, #stuff)
		swapFromContent = stuff[swapFrom]
		swapToContent = stuff[swapTo]
		stuff[swapTo] = swapFromContent
		stuff[swapFrom] = swapToContent
	end
end

function generate()
	shuffle()
	local txt="\n"
	for i=1, math.random(1,2) do
		txt = txt .. stuff[i] .. "\n\n"
	end
        return txt
end

titles = {
"Order Now And You Also Get A Attack Nose",
"The Care And Feeding Of Your Sleeping Bicycle",
"Now Anybody Can Make President",
"'Star Wars' As Written By A Princess",
"What I Learned From An Elephant",
"Today, The World - Tomorrow, The Mixed-Up Dice",
"The Funniest Joke About A Grandmother's Tree",
"Dr. Jekyll And Mr. Bear",
"Christmas Shopping For A Racing Ark",
"Once Upon A Guardian Dinosaur",
"The Mystery Of Lego Pirate",
"Thomas Edison Invents The Guardian Rollercoaster",
"Today, The World - Tomorrow, The Complicated Moonlight",
"'Star Wars' As Written By A Scary Bat",
"Anatomy Of A Funny Day",
"On The Trail Of The Electric Desk",
"My Coach Is A New, Improved Bear",
"My Son, The Lost Banana",
"The Olympic Competition Won By An Automatic Monkey",
"The Olympic Competition Won By An Purple Bicycle",
"Marco Polo Discovers The Complicated Spoon",
"The Mystery Of Horse",
"Now Anybody Can Make Miniature Nose",
"My Daughter, The Spoon",
"Dental Surgery On A Desert Elephant",
"On The Trail Of The Automatic Giant",
"What I Learned From An Football",
"Now Anybody Can Make Attack Wolf",
"Avast, Me Invisible Twin Fish",
"Around The World With A Blustery Banana",
"Mr. McMullet, The Flying Tree",
"Visit Fun World And See The Rare Recycled Clown",
"Wyoming Jones And The Secret Twin Canadian Bat",
"Wyoming Jones And The Clown",
"Marco Polo Discovers The Accountant",
"Playing Poker With A Tree",
"Barney And The Rollercoaster",
"Today, The World - Tomorrow, The Purple Money",
"I'm My Own Desert Friend",
"I Have To Write About My Blustery Funny Nose",
"Once Upon A Complicated Duck",
"On The Trail Of The Lost Chicken",
"Way Out West With The Green Giant",
"No Man Is A Monkey",
"Where To Meet An Impossible Friend",
"Barney And The Hungry Desk",
"Way Out West With The Furry Super Wolf",
"The Mystery Of Lego Chicken",
"I Rode Friendly Grandmother's Mixed-Up Chocolate",
"Christmas Shopping For A New, Improved Ark",
"The Funniest Joke About A Scary Ark",
"Where To Meet An Chicken",
"Once Upon Tired Water",
"Always Share Your Tyrannosaurus Moonlight",
"Marco Polo Discovers The Guardian Bicycle",
"Training Your Proud Football",
"Sherlock Holmes And The Complicated Friendly Doctor",
"Christmas Shopping For A Hairy Dance",
"We Elected A Secret Tree",
"What It's Like Inside A Furry Desert Nose",
}

