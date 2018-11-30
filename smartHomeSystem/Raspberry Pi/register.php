<?php
	if(isset($_POST["Token"])){
	
		$token = $_POST["Token"];
		include_once 'config.php';
		$conn = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME);
		$query = "INSERT INTO users(Token) Values ('$token') ON DUPLICATE KEY UPDATE Token = '$token'; ";
		$result = mysqli_query($conn, $query);
		echo $result;
		mysqli_close($conn);
		echo "iii";
	}
	echo "yyy";
?>