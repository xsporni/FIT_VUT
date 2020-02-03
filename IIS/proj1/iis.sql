-- phpMyAdmin SQL Dump
-- version 4.9.0.1
-- https://www.phpmyadmin.net/
--
-- Hostiteľ: 127.0.0.1:3306
-- Čas generovania: Po 04.Nov 2019, 11:46
-- Verzia serveru: 10.4.6-MariaDB
-- Verzia PHP: 7.3.9

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Databáza: `iis`
--

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `courses`
--

CREATE TABLE `courses` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `abrv` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` mediumtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `type` enum('web design','SEO','web development') COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `tags` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `price` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `status` enum('pending','accepted','declined') COLLATE utf8mb4_unicode_ci NOT NULL,
  `guarantor_id` bigint(20) UNSIGNED DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `courses`
--

INSERT INTO `courses` (`id`, `name`, `abrv`, `description`, `type`, `tags`, `price`, `created_at`, `updated_at`, `status`, `guarantor_id`) VALUES
(1, 'HTML and CSS', 'HAC', 'Learn to create simple static websites with two basic languages that web is build with, HTML and CSS. You will learn structure of websites with HTML and how to make them pretty with CSS.', 'web design', '[\"HTML\", \"CSS\", \"web design\", \"beginner\", \"101\"]', 15, NULL, NULL, 'accepted', 1),
(2, 'Web Design with Photoshop', 'WDP', 'Learn to design stunning websites with Photoshop. It is a industry standard program that is tool of many designers. You\'ll learn wireframing, basics of photoshop and you\'ll create real-world project.', 'web design', '[\"web design\", \"photoshop\", \"101\", \"beginners\", \"wireframing\"]', 20, NULL, '2019-10-30 19:16:03', 'declined', 1),
(5, 'Information Systems', 'IIS', 'This is course for students who want to learn basics of Information Systems concepts.', NULL, NULL, 90, '2019-10-29 15:58:38', '2019-10-31 13:07:54', 'accepted', 6),
(7, 'C Programming', 'IJC', 'ip link set dev v2_def upip link set dev v2_def upip link set dev v2_def upip link set dev v2_def upip link set dev v2_def upip link set dev v2_def upip link set dev v2_def upip link set dev v2_def up', NULL, NULL, 10, '2019-10-30 21:11:50', '2019-10-30 21:12:10', 'accepted', 6);

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `course_lecturers`
--

CREATE TABLE `course_lecturers` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `course_id` bigint(20) UNSIGNED NOT NULL,
  `user_id` bigint(20) UNSIGNED NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `course_lecturers`
--

INSERT INTO `course_lecturers` (`id`, `course_id`, `user_id`, `created_at`, `updated_at`) VALUES
(1, 1, 4, '2019-10-28 08:37:49', '2019-10-28 08:37:49'),
(2, 1, 5, '2019-10-28 08:38:42', '2019-10-28 08:38:42'),
(3, 5, 4, NULL, NULL),
(4, 5, 6, NULL, NULL),
(5, 7, 4, NULL, NULL),
(6, 5, 7, NULL, NULL);

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `course_students`
--

CREATE TABLE `course_students` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `course_id` bigint(20) UNSIGNED NOT NULL,
  `user_id` bigint(20) UNSIGNED NOT NULL,
  `points` tinyint(4) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `status` enum('accepted','pending','declined') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'pending'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `course_students`
--

INSERT INTO `course_students` (`id`, `course_id`, `user_id`, `points`, `created_at`, `updated_at`, `status`) VALUES
(2, 1, 6, 0, NULL, NULL, 'accepted'),
(9, 1, 7, 0, NULL, NULL, 'accepted'),
(15, 7, 7, 0, NULL, NULL, 'accepted'),
(16, 7, 6, 0, NULL, NULL, 'accepted'),
(17, 5, 8, 0, NULL, NULL, 'accepted');

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `failed_jobs`
--

CREATE TABLE `failed_jobs` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `connection` text COLLATE utf8mb4_unicode_ci NOT NULL,
  `queue` text COLLATE utf8mb4_unicode_ci NOT NULL,
  `payload` longtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `exception` longtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `failed_at` timestamp NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `migrations`
--

CREATE TABLE `migrations` (
  `id` int(10) UNSIGNED NOT NULL,
  `migration` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `batch` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `migrations`
--

INSERT INTO `migrations` (`id`, `migration`, `batch`) VALUES
(5, '2014_10_12_000000_create_users_table', 1),
(6, '2014_10_12_100000_create_password_resets_table', 1),
(7, '2019_08_19_000000_create_failed_jobs_table', 1),
(8, '2019_10_02_200843_create_courses_table', 1),
(9, '2019_10_03_100108_create_admins_table', 2),
(10, '2019_10_08_083949_add_course_ids_to_user', 3),
(14, '2019_10_09_084551_add_status_to_courses', 4),
(15, '2019_10_09_090302_create_time_slots_table', 4),
(16, '2019_10_09_092935_create_evaluations_table', 5),
(17, '2019_10_09_093639_create_rooms_table', 6),
(18, '2019_10_09_094340_add_foto_and_role_to_users', 6),
(21, '2019_10_09_095635_add_time_interval_to_time_slots', 7),
(22, '2019_10_14_170121_add_room_id_and_time_to_time_slots', 8),
(24, '2019_10_14_170933_courses_and_lecturers', 9),
(25, '2019_10_14_174245_add_guarantor_id_to_courses', 10),
(29, '2019_10_14_183537_add_courses_users_table', 11),
(30, '2019_10_28_092809_create_course_lecturers_table', 12),
(31, '2019_10_28_093259_create_course_students_table', 13),
(32, '2019_10_28_183107_add_points_to_time_slots_table', 14),
(33, '2019_10_28_191012_adding_columns_time_slots_table', 15),
(34, '2019_10_28_195602_adding_status_to_course_students_table', 16);

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `password_resets`
--

CREATE TABLE `password_resets` (
  `email` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `token` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `rooms`
--

CREATE TABLE `rooms` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `address` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `door_number` smallint(6) NOT NULL,
  `type` enum('salle','laboratory') COLLATE utf8mb4_unicode_ci NOT NULL,
  `capacity` smallint(6) NOT NULL,
  `equipment` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `time_slots`
--

CREATE TABLE `time_slots` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `course_id` bigint(20) UNSIGNED NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `room_id` bigint(20) UNSIGNED DEFAULT NULL,
  `type` enum('lecture','exercise','test','homework','other') COLLATE utf8mb4_unicode_ci NOT NULL,
  `date` date NOT NULL,
  `points` tinyint(4) DEFAULT NULL,
  `term_start` timestamp NULL DEFAULT NULL,
  `term_end` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `time_slots`
--

INSERT INTO `time_slots` (`id`, `name`, `course_id`, `created_at`, `updated_at`, `room_id`, `type`, `date`, `points`, `term_start`, `term_end`) VALUES
(1, 'Uvod do predmetu', 1, '2019-10-28 18:17:48', '2019-10-28 18:17:48', NULL, 'lecture', '2019-10-28', 10, NULL, NULL),
(2, 'Intro', 5, '2019-10-31 22:23:54', '2019-10-31 22:23:54', NULL, 'lecture', '2019-11-12', 1, NULL, NULL),
(3, 'WPF', 5, '2019-10-31 22:26:06', '2019-10-31 22:26:06', NULL, 'exercise', '2019-11-12', NULL, NULL, NULL),
(4, 'Basic WPF structure with forms', 5, '2019-10-31 22:29:26', '2019-10-31 22:29:26', NULL, 'test', '2019-11-15', 10, NULL, NULL),
(5, 'Zaverecna skuska', 5, '2019-11-01 14:39:40', '2019-11-01 14:39:40', NULL, 'test', '2019-11-29', 30, NULL, NULL);

-- --------------------------------------------------------

--
-- Štruktúra tabuľky pre tabuľku `users`
--

CREATE TABLE `users` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `email` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `email_verified_at` timestamp NULL DEFAULT NULL,
  `password` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `remember_token` varchar(100) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `photo` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `role` enum('admin','director','guarantor','lecturer','student','unregistered') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'student'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Sťahujem dáta pre tabuľku `users`
--

INSERT INTO `users` (`id`, `name`, `email`, `email_verified_at`, `password`, `remember_token`, `created_at`, `updated_at`, `photo`, `role`) VALUES
(1, 'John Doe', 'johndoe@gmail.com', NULL, '$2y$10$5v8fdKJoNQ8JX58/2eZkYOdC63Iy/J/qpXwXCy5ezbIy7f6r6krsm', NULL, '2019-10-03 08:23:08', '2019-10-03 08:23:08', '', 'guarantor'),
(3, 'Alex', 'alex@gmail.com', NULL, '$2y$10$mTTaFf4G6d8tBZxYmAicL.hyKVH3KOj7YN3LERvjKcExQV194NRHK', NULL, '2019-10-08 12:07:06', '2019-10-08 12:07:06', '', 'admin'),
(4, 'Ales Smrcka', 'smrcka@gmail.com', NULL, '$2y$10$hnvu.soXCblQ2vJZPIgWBurhBN.HT9WFyd20xhVCPhxptkLO1B.Fe', NULL, '2019-10-14 15:58:56', '2019-10-14 15:58:56', 'c/asdnj/', 'lecturer'),
(5, 'Dano Weis', 'weisko@gmail.com', NULL, '$2y$10$t/Vxj3sko/mr3E3zd0yGrO13Z7Df/m7ov4z6u6HQ5kTGylfIXp4c.', NULL, '2019-10-14 16:01:35', '2019-10-14 16:01:35', 'c/asd/', 'student'),
(6, 'Igor Mjasojedov', 'mjasojedov@schoolio.edu', NULL, '$2y$10$u1nQCKVIy1gOnTzc5MfrQe1QKMzFkKtRivMn36IPi6xvVXG8zVWVK', NULL, '2019-10-28 06:57:07', '2019-10-28 06:57:07', '/sdf/sdf', 'director'),
(7, 'Janko', 'janko@schoolio.edu', NULL, '$2y$10$nFBcZTDCwgRU2TT0iCYQ8.i8Oo/v4vYXuV0.D3jvfTiKCIHz4V9ki', NULL, '2019-10-29 19:04:16', '2019-10-29 19:04:16', NULL, 'lecturer'),
(8, 'Anatolij Mjasojedov', 'anatolij@gmail.com', NULL, '$2y$10$kj7JKuw..I2QCJUfWEnJfO9pW1kJk2i0Bg0x4DKfigZaX9e0w7DHC', NULL, '2019-11-01 14:47:06', '2019-11-01 14:47:06', NULL, 'student');

--
-- Kľúče pre exportované tabuľky
--

--
-- Indexy pre tabuľku `courses`
--
ALTER TABLE `courses`
  ADD PRIMARY KEY (`id`),
  ADD KEY `courses_guarantor_id_foreign` (`guarantor_id`);

--
-- Indexy pre tabuľku `course_lecturers`
--
ALTER TABLE `course_lecturers`
  ADD PRIMARY KEY (`id`),
  ADD KEY `course_lecturers_course_id_foreign` (`course_id`),
  ADD KEY `course_lecturers_user_id_foreign` (`user_id`);

--
-- Indexy pre tabuľku `course_students`
--
ALTER TABLE `course_students`
  ADD PRIMARY KEY (`id`),
  ADD KEY `course_students_course_id_foreign` (`course_id`),
  ADD KEY `course_students_user_id_foreign` (`user_id`);

--
-- Indexy pre tabuľku `failed_jobs`
--
ALTER TABLE `failed_jobs`
  ADD PRIMARY KEY (`id`);

--
-- Indexy pre tabuľku `migrations`
--
ALTER TABLE `migrations`
  ADD PRIMARY KEY (`id`);

--
-- Indexy pre tabuľku `password_resets`
--
ALTER TABLE `password_resets`
  ADD KEY `password_resets_email_index` (`email`);

--
-- Indexy pre tabuľku `rooms`
--
ALTER TABLE `rooms`
  ADD PRIMARY KEY (`id`);

--
-- Indexy pre tabuľku `time_slots`
--
ALTER TABLE `time_slots`
  ADD PRIMARY KEY (`id`),
  ADD KEY `time_slots_course_id_foreign` (`course_id`),
  ADD KEY `time_slots_room_id_foreign` (`room_id`);

--
-- Indexy pre tabuľku `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `users_email_unique` (`email`);

--
-- AUTO_INCREMENT pre exportované tabuľky
--

--
-- AUTO_INCREMENT pre tabuľku `courses`
--
ALTER TABLE `courses`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=10;

--
-- AUTO_INCREMENT pre tabuľku `course_lecturers`
--
ALTER TABLE `course_lecturers`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT pre tabuľku `course_students`
--
ALTER TABLE `course_students`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=18;

--
-- AUTO_INCREMENT pre tabuľku `failed_jobs`
--
ALTER TABLE `failed_jobs`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pre tabuľku `migrations`
--
ALTER TABLE `migrations`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=35;

--
-- AUTO_INCREMENT pre tabuľku `rooms`
--
ALTER TABLE `rooms`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pre tabuľku `time_slots`
--
ALTER TABLE `time_slots`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;

--
-- AUTO_INCREMENT pre tabuľku `users`
--
ALTER TABLE `users`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- Obmedzenie pre exportované tabuľky
--

--
-- Obmedzenie pre tabuľku `courses`
--
ALTER TABLE `courses`
  ADD CONSTRAINT `courses_guarantor_id_foreign` FOREIGN KEY (`guarantor_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;

--
-- Obmedzenie pre tabuľku `course_lecturers`
--
ALTER TABLE `course_lecturers`
  ADD CONSTRAINT `course_lecturers_course_id_foreign` FOREIGN KEY (`course_id`) REFERENCES `courses` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `course_lecturers_user_id_foreign` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;

--
-- Obmedzenie pre tabuľku `course_students`
--
ALTER TABLE `course_students`
  ADD CONSTRAINT `course_students_course_id_foreign` FOREIGN KEY (`course_id`) REFERENCES `courses` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `course_students_user_id_foreign` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;

--
-- Obmedzenie pre tabuľku `time_slots`
--
ALTER TABLE `time_slots`
  ADD CONSTRAINT `time_slots_course_id_foreign` FOREIGN KEY (`course_id`) REFERENCES `courses` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `time_slots_room_id_foreign` FOREIGN KEY (`room_id`) REFERENCES `time_slots` (`id`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
