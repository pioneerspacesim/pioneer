local r = function (t, rand) return t[rand:Integer(1,#t)] end

NameGen = {
	firstNames = {
		male   = {},
		female = {},
	},
	surnames = {},
    planetFormats = {},
    starportFormats = {},

    FullName = function (isfemale, rand)
		if not rand then rand = Engine.rand end

        local firstname
        if isfemale then
			firstname = r(NameGen.firstNames.female, rand)
		else
            firstname = r(NameGen.firstNames.male, rand)
		end

		return firstname .. " " .. NameGen.Surname(rand)
    end,

	Surname = function (rand)
		if not rand then rand = Engine.rand end

		return r(NameGen.surnames, rand)
	end,

	BodyName = function (body, rand)
		if body.type == "STARPORT_ORBITAL" then
			return r(NameGen.surnames, rand) .. " Spaceport"
		end

		if body.type == "STARPORT_SURFACE" then
			return r(NameGen.surnames, rand) .. " Starport"
		end

		print("such fun")
		return "such fun"

	end
}

NameGen.firstNames.male = {
	'Bob', 'Stephen', 'Paul', 'Andrew', 'David', 'Mark', 'John', 'Robert',
	'James', 'Christopher', 'Michael', 'Tom', 'Thomas', 'Jim', 'George',
	'Arthur', 'Henry', 'Albert', 'Adam', 'Adrian', 'Alan', 'Alex', 'Alistair',
	'Ben', 'Barry', 'Bernard', 'Alexander', 'Bill', 'Brian', 'Bruce', 'Byron',
	'Carl', 'Clive', 'Dan', 'Dave', 'Derek', 'Dennis', 'Geoffrey', 'Gary',
	'Graham', 'Guy', 'Herbert', 'Ian', 'Jackie', 'Jake', 'Jan', 'Jeremy',
	'Jimmy', 'Josh', 'Keith', 'Kenneth', 'Leon', 'Lance', 'Malcolm', 'Matthew',
	'Neil', 'Nicholas', 'Non', 'Patrick', 'Peter', 'Philip', 'Ralph',
	'Richard', 'Roger', 'Robert', 'Roy', 'Simon', 'Steve', 'Tim', 'Tony',
	'Tristram', 'Vaughan', 'Wayne', 'William', 'Jose', 'Antonio', 'Manuel',
	'Francisco', 'Juan', 'Pedro', 'Jose Luis', 'Jesus', 'Angel', 'Luis',
	'Miguel', 'Rafael', 'Jose Antonio', 'Jose Maria', 'Fernando', 'Vincente',
	'Jose Manuel', 'Ramon', 'Carlos', 'Francisco Javier', 'Joaquin', 'Enrique',
	'Juan Jose', 'Andres', 'Santiago', 'Emilio', 'Javier', 'Julian',
	'Juan Antonio', 'Felix', 'Alfonso', 'Juan Carlos', 'Salvador', 'Tomas',
	'Eduardo', 'Agustin', 'Mariano', 'Ricardo', 'Pablo', 'Alberto',
	'Juan Manuel', 'Domingo', 'Jaime', 'Ignacio', 'Diego', 'Gregorio',
	'Alejandro', 'Felipe', 'Daniel', 'Jose Ramon', 'Jorge', 'Alfredo',
}

NameGen.firstNames.female = {
	'Jen', 'Steph', 'Hannah', 'Alison', 'Amanda', 'Angela', 'Ann', 'Anne',
	'Audrey', 'Barbara', 'Beryl', 'Betty', 'Beth', 'Brenda', 'Carol',
	'Caroline', 'Catherine', 'Cathy', 'Celia', 'Cheryl', 'Christine', 'Claire',
	'Daphne', 'Diana', 'Dorothy', 'Elise', 'Elaine', 'Edith', 'Emma', 'Ella',
	'Erica', 'Esther', 'Eva', 'Fran', 'Frances', 'Fiona', 'Gill', 'Gillian',
	'Hazel', 'Heather', 'Helen', 'Hilary', 'Irena', 'Isobel', 'Jane', 'Janet',
	'Janice', 'Jeanette', 'Jenny', 'Jennifer', 'Jill', 'Jo', 'Joan', 'Joanna',
	'Joy', 'Juliette', 'Judy', 'Julia', 'Karen', 'Kate', 'Kathy', 'Katherine',
	'Laura', 'Linda', 'Lisa', 'Louise', 'Lucy', 'Luna', 'Maggie', 'Margaret',
	'Maria', 'Mariam', 'Marilyn', 'Marion', 'Maureen', 'Molly', 'Miriam',
	'Morag', 'Monica', 'Nat', 'Natalia', 'Nicola', 'Pam', 'Pamela', 'Patricia',
	'Pauline', 'Penny', 'Rachel', 'Rose', 'Rosemary', 'Rosie', 'Sally',
	'Sandra', 'Sarah', 'Stella', 'Sue', 'Sally', 'Susan', 'Susanne', 'Suzy',
	'Tracy', 'Valerie', 'Vicky', 'Vivian', 'Violet', 'Wendy', 'Yvonne', 'Zoe',
	'Maria', 'Carmen', 'Josefa', 'Isabel', 'Dolores', 'Francisca', 'Antonia',
	'Pilar', 'Ana Maria', 'Ana', 'Maria Luisa', 'Mercedes', 'Manuela', 'Juana',
	'Rosario', 'Teresa', 'Maria Jose', 'Margarita', 'Maria Angeles', 'Angeles',
	'Maria Pilar',
}

NameGen.surnames = {
	'Morton', 'Butcher', 'Davidson', 'Edwards', 'Findley', 'Hughes',
	'Martinez', 'Ibarrez', 'Smith', 'Johnson', 'Williams', 'Jones', 'Brown',
	'Davis', 'Miller', 'Wilson', 'Moore', 'Taylor', 'Anderson', 'Thomas',
	'Jackson', 'White', 'Harris', 'Martin', 'Thompson', 'Garcia', 'Robinson',
	'Glark', 'Rodriguez', 'Lewis', 'Lee', 'Walker', 'Hall', 'Allen', 'Young',
	'Hernandez', 'King', 'Wright', 'Lopez', 'Hill', 'Scott', 'Green', 'Adams',
	'Baker', 'Gonzalez', 'Nelson', 'Carter', 'Mitchell', 'Perez', 'Roberts',
	'Turner', 'Phillips', 'Campbell', 'Parker', 'Evans', 'Edwards', 'Collin',
	'Stewart', 'Sanchez', 'Morris', 'Rogers', 'Reed', 'Cook', 'Morgan',
	'Bell', 'Murphy', 'Bailey', 'Rivera', 'Cooper', 'Richardson', 'Cox',
	'Howard', 'Ward', 'Torres', 'Peterson', 'Gray', 'Ramirez', 'James',
	'Watson', 'Brooks', 'Kelly', 'Sanders', 'Price', 'Bennet', 'Wood',
	'Barnes', 'Ross', 'Henderson', 'Coleman', 'Jenkins', 'Perry', 'Powell',
	'Long', 'Patterson', 'Hughes', 'Flores', 'Washington', 'Butler',
	'Simmons', 'Foster', 'Gonzales', 'Bryant', 'Alexander', 'Russel',
	'Griffin', 'Diaz', 'Myers', 'Ford', 'Hamilton', 'Graham', 'Sullivan',
	'Wallace', 'Woods', 'Cole', 'West', 'Jordan', 'Owens', 'Reynolds',
	'Fisher', 'Ellis', 'Harrison', 'Gibson', 'Mcdonald', 'Cruz', 'Marshall',
	'Ortiz', 'Gomez', 'Murray', 'Freeman', 'Wells', 'Webb', 'Simpson',
	'Stevens', 'Tucker', 'Porter', 'Hunter', 'Hicks', 'Crawford', 'Henry',
	'Boyd', 'Mason', 'Morales', 'Kennedy', 'Warren', 'Dixon', 'Ramos',
	'Reyes', 'Burns', 'Gordon', 'Shaw', 'Holmes', 'Rice', 'Robertson', 'Hunt',
	'Black', 'Daniels', 'Palmer', 'Mills', 'Nichols', 'Grant', 'Knight',
	'Ferguson', 'Rose', 'Stone', 'Hawkins', 'Dunn', 'Hudson', 'Spencer',
	'Gardner', 'Stephens', 'Payne', 'Pierce', 'Berry', 'Matthews', 'Arnold',
	'Wagner', 'Willis', 'Ray', 'Watkins', 'Olson', 'Carrol', 'Duncan',
	'Snyder', 'Hard', 'Cunningham', 'Lane', 'Andrews', 'Ruiz', 'Harper',
	'Fox', 'Riley', 'Armstrong', 'Carpenter', 'Weaver', 'Greene', 'Lawrence',
	'Elliott', 'Chavez', 'Sims', 'Austin', 'Peters', 'Kelley', 'Franklin',
	'Lawson', 'Fields', 'Gutierrez', 'Ryan', 'Schmidt', 'Carr', 'Vasquez',
	'Castillo', 'Wheeler', 'Chapman', 'Oliver', 'Montgomery', 'Richards',
	'Williamson', 'Johnston', 'Banks', 'Meyer', 'Bishop', 'Mccoy', 'Howell',
	'Alvarez', 'Morrison', 'Hansen', 'Fernandez', 'Garza', 'Harvey', 'Little',
	'Burton', 'Stanley', 'George', 'Jacobs', 'Reid', 'Kim', 'Fuller', 'Lynch',
	'Dean', 'Gilbert', 'Romero', 'Welch', 'Larson', 'Watts', 'Miles', 'Lucas',
	'Castropena', 'Rhodes', 'Hardy', 'Santiago', 'Powers', 'Schultz', 'Munoz',
	'Chandler', 'Wolfe', 'Schneider', 'Valdez', 'Salazar', 'Warner', 'Tate',
	'Moss', 'Vega', 'Aguilar', 'Reese', 'Townsend', 'Goodwin', 'Rowe',
	'Newton', 'Maxwell', 'Gibbs', 'Wise', 'Zimmerman', 'Wong', 'Vazquez',
	'Espinoza', 'Sawyer', 'Jordan', 'Guerra', 'Miranda', 'Atkinson', 'Campos',
	'Sloan', 'Juarez', 'Weiss', 'Nixon', 'Hurst', 'Lowery', 'Farrell',
	'Maynard', 'Walter', 'Foley', 'Rivers', 'Walls', 'Estes', 'Morse',
	'Sheppard', 'Weeks', 'Bean', 'Barron', 'Livingston', 'Middleton',
	'Spears', 'Branch', 'Blevins', 'Chen', 'Kerr', 'Mcconnell', 'Harding',
	'Ashley', 'Solis', 'Herman', 'Frost', 'Giles', 'Blackburn', 'William',
	'Pennington', 'Woodward', 'Finley', 'Mcintosh', 'Koch', 'Best', 'Solomon',
	'Mccullough', 'Dudley', 'Nolan', 'Blanchard', 'Rivas', 'Brennan', 'Mejia',
	'Kane', 'Joyce', 'Buckley', 'Haley', 'Moon', 'Mcmillan', 'Crosby', 'Berg',
	'Dotson', 'Mays', 'Roach', 'Church', 'Chan', 'Richmond', 'Meadows',
	'Faulkner', 'Oneill', 'Knapp', 'Kline', 'Barry', 'Ochoa', 'Jacobson',
	'Gay', 'Avery', 'Hendricks', 'Horne', 'Shepard', 'Herbert', 'Cherry',
	'Cardenas', 'Holman', 'Donaldson', 'Terrell', 'Morin', 'Fuentes',
	'Tillman', 'Sanford', 'Bentley', 'Peck', 'Key', 'Salas', 'Rollins',
	'Gamble', 'Dickson', 'Santana', 'Cabrera', 'Cervantes', 'Howe', 'Hinton',
	'Hurley', 'Spence', 'Zamora', 'Yang', 'Mcneil', 'Suarez', 'Case',
	'Pretty', 'Gould', 'Mcfarland', 'Sampson', 'Carver', 'Bray', 'Rosario',
	'Macdonald', 'Stout', 'Hester', 'Melendez', 'Dillon', 'Farley', 'Hopper',
	'Galloway', 'Potts', 'Bernard', 'Joyner', 'Stein', 'Aguirre', 'Osborn',
	'Mercer', 'Bender', 'Franco', 'Rowland', 'Sykes', 'Benjamin', 'Travis',
	'Pickett', 'Crane', 'Sears', 'Mayo', 'Dunlap', 'Hayden', 'Wilder',
	'Mckay', 'Ewing', 'Cooley', 'Bonner', 'Cotton', 'Stark', 'Ferrel',
	'Cantrell', 'Fulton', 'Calderon', 'Rosa', 'Hooper', 'Mullen', 'Burch',
	'Fry', 'Riddle', 'Levy', 'Duke', 'Odonnell', 'Britt', 'Frederick',
	'Daugherty', 'Berger', 'Dillard', 'Alston', 'Jarvis', 'Frye', 'Riggs',
	'Chaney', 'Odom', 'Fitzpatrick', 'Valenzuela', 'Merrill', 'Mayer',
	'Alford', 'Mcpherson', 'Acevedo', 'Donovan', 'Albert', 'Cote', 'Reilly',
	'Compton', 'Raymond', 'Mooney', 'Mcgowan', 'Craft', 'Cleveland',
	'Clemons', 'Wynn', 'Nielsen', 'Baird', 'Stanton', 'Snider', 'Rosales',
	'Bright', 'Witt', 'Hays', 'Holden', 'Soto', 'Slater', 'Kinney',
	'Clements', 'Hahn', 'Emerson', 'Conrad', 'Burks', 'Lancaster', 'Justice',
	'Tyson', 'Sharpe', 'Whitfield', 'Talley', 'Macias', 'Irwin', 'Burris',
	'Mccray', 'Madden', 'Goff', 'Bolton', 'Mcfadden', 'Levine', 'Byers',
	'Kirkland', 'Kidd', 'Carney', 'Mcleod', 'Hendrix', 'Sosa', 'Rasmussen',
	'Valencia', 'De La Cruz', 'Forbes', 'Guthrie', 'Wooten', 'Huber',
	'Barlow', 'Boyle', 'Buckner', 'Rocha', 'Langley', 'Cooke', 'Velazquez',
	'Noel', 'Vang', 'Li', 'Wang', 'Chang', 'Liu', 'Chen', 'Yang', 'Huang',
	'Chao', 'Chou', 'Wu', 'Hsu', 'Sun', 'Chu', 'Ma', 'Hu', 'Kuo', 'Lin',
	'Kao', 'Liang', 'Cheng', 'Lo', 'Sung', 'Hsieh', 'Tang', 'Han', 'Tsao',
	'Hsu', 'Teng', 'Hsiao', 'Feng', 'Tseng', 'Cheng', 'Tsaipeng', 'Pan',
	'Yuan', 'Yu', 'Yung', 'Su', 'Wei', 'Chiang', 'Ting', 'Shen', 'Chiang',
	'Fan', 'Chung', 'Wang', 'Tai', 'Liao', 'Fang', 'Chin', 'Hsai', 'Chia',
	'Tsou', 'Shih', 'Hsiung', 'Yen', 'Hou', 'Lei', 'Lung', 'Tuan', 'Hao',
	'Shao', 'Shih', 'Mao', 'Wan', 'Kang', 'Yen', 'Yin', 'Shih', 'Niu', 'Hung',
	'Kung',
}
