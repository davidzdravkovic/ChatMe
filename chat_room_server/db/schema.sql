--
-- PostgreSQL database dump
--

-- Dumped from database version 17.6
-- Dumped by pg_dump version 17.5

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: media_state; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE public.media_state AS ENUM (
    'TEMP',
    'READY'
);


--
-- Name: message_status; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE public.message_status AS ENUM (
    'READY',
    'PENDING_MEDIA',
    'FAILED'
);


SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: chat_participants; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.chat_participants (
    id integer NOT NULL,
    chatroom_id integer NOT NULL,
    user_id integer NOT NULL,
    joined_at timestamp without time zone DEFAULT now()
);


--
-- Name: chat_participants_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.chat_participants_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: chat_participants_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.chat_participants_id_seq OWNED BY public.chat_participants.id;


--
-- Name: chat_seen; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.chat_seen (
    chatroom_id integer NOT NULL,
    user_id integer NOT NULL,
    last_seen_message_id integer NOT NULL,
    seen_at timestamp with time zone DEFAULT now()
);


--
-- Name: chatrooms; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.chatrooms (
    id integer NOT NULL,
    type character varying(20) NOT NULL,
    created_at timestamp without time zone DEFAULT now()
);


--
-- Name: chatrooms_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.chatrooms_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: chatrooms_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.chatrooms_id_seq OWNED BY public.chatrooms.id;


--
-- Name: media; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.media (
    media_id bigint NOT NULL,
    owner_user_id bigint NOT NULL,
    state public.media_state NOT NULL,
    created_at timestamp without time zone DEFAULT now() NOT NULL,
    finalized_at timestamp without time zone,
    mime_type text NOT NULL,
    file_size_bytes bigint NOT NULL
);


--
-- Name: media_media_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.media_media_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: media_media_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.media_media_id_seq OWNED BY public.media.media_id;


--
-- Name: message_reactions; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.message_reactions (
    message_id integer NOT NULL,
    user_id integer NOT NULL,
    reaction character varying(32) NOT NULL,
    created_at timestamp without time zone DEFAULT now()
);


--
-- Name: messages; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.messages (
    message_id integer NOT NULL,
    chatroom_id integer NOT NULL,
    sender_id integer NOT NULL,
    content text,
    created_at timestamp without time zone DEFAULT now(),
    media_id bigint,
    status public.message_status DEFAULT 'READY'::public.message_status NOT NULL,
    reply_to_message_id integer
);


--
-- Name: messages_message_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.messages_message_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: messages_message_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.messages_message_id_seq OWNED BY public.messages.message_id;


--
-- Name: private_chats; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.private_chats (
    id integer NOT NULL,
    user1_id integer NOT NULL,
    user2_id integer NOT NULL,
    chatroom_id integer,
    CONSTRAINT private_chats_user_order CHECK ((user1_id < user2_id))
);


--
-- Name: private_chats_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.private_chats_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: private_chats_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.private_chats_id_seq OWNED BY public.private_chats.id;


--
-- Name: users; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.users (
    user_id integer NOT NULL,
    username character varying(50) NOT NULL,
    password text NOT NULL,
    email character varying(100),
    created_at timestamp without time zone DEFAULT now(),
    name text,
    profile_image_updated_at timestamp without time zone,
    profile_image_media_id bigint,
    last_active_at timestamp with time zone DEFAULT now()
);


--
-- Name: users_user_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.users_user_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: users_user_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.users_user_id_seq OWNED BY public.users.user_id;


--
-- Name: chat_participants id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_participants ALTER COLUMN id SET DEFAULT nextval('public.chat_participants_id_seq'::regclass);


--
-- Name: chatrooms id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chatrooms ALTER COLUMN id SET DEFAULT nextval('public.chatrooms_id_seq'::regclass);


--
-- Name: media media_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.media ALTER COLUMN media_id SET DEFAULT nextval('public.media_media_id_seq'::regclass);


--
-- Name: messages message_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages ALTER COLUMN message_id SET DEFAULT nextval('public.messages_message_id_seq'::regclass);


--
-- Name: private_chats id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats ALTER COLUMN id SET DEFAULT nextval('public.private_chats_id_seq'::regclass);


--
-- Name: users user_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users ALTER COLUMN user_id SET DEFAULT nextval('public.users_user_id_seq'::regclass);


--
-- Name: chat_participants chat_participants_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_participants
    ADD CONSTRAINT chat_participants_pkey PRIMARY KEY (id);


--
-- Name: chat_participants chat_participants_unique; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_participants
    ADD CONSTRAINT chat_participants_unique UNIQUE (chatroom_id, user_id);


--
-- Name: chat_seen chat_seen_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_seen
    ADD CONSTRAINT chat_seen_pkey PRIMARY KEY (chatroom_id, user_id);


--
-- Name: chatrooms chatrooms_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chatrooms
    ADD CONSTRAINT chatrooms_pkey PRIMARY KEY (id);


--
-- Name: media media_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.media
    ADD CONSTRAINT media_pkey PRIMARY KEY (media_id);


--
-- Name: message_reactions message_reactions_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.message_reactions
    ADD CONSTRAINT message_reactions_pkey PRIMARY KEY (message_id, user_id);


--
-- Name: messages messages_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_pkey PRIMARY KEY (message_id);


--
-- Name: private_chats private_chats_chatroom_id_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_chatroom_id_key UNIQUE (chatroom_id);


--
-- Name: private_chats private_chats_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_pkey PRIMARY KEY (id);


--
-- Name: private_chats private_chats_unique_pair; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_unique_pair UNIQUE (user1_id, user2_id);


--
-- Name: users users_email_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_email_key UNIQUE (email);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (user_id);


--
-- Name: users users_username_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_username_key UNIQUE (username);


--
-- Name: idx_message_reactions_message_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_message_reactions_message_id ON public.message_reactions USING btree (message_id);


--
-- Name: idx_messages_chatroom_created; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_messages_chatroom_created ON public.messages USING btree (chatroom_id, created_at);


--
-- Name: idx_messages_chatroom_message_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_messages_chatroom_message_id ON public.messages USING btree (chatroom_id, message_id);


--
-- Name: idx_messages_last_message; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_messages_last_message ON public.messages USING btree (chatroom_id, created_at DESC) WHERE (status = 'READY'::public.message_status);


--
-- Name: chat_participants chat_participants_chatroom_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_participants
    ADD CONSTRAINT chat_participants_chatroom_id_fkey FOREIGN KEY (chatroom_id) REFERENCES public.chatrooms(id) ON DELETE CASCADE;


--
-- Name: chat_participants chat_participants_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_participants
    ADD CONSTRAINT chat_participants_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(user_id) ON DELETE CASCADE;


--
-- Name: chat_seen chat_seen_chatroom_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_seen
    ADD CONSTRAINT chat_seen_chatroom_id_fkey FOREIGN KEY (chatroom_id) REFERENCES public.chatrooms(id);


--
-- Name: chat_seen chat_seen_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_seen
    ADD CONSTRAINT chat_seen_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(user_id);


--
-- Name: media fk_media_owner; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.media
    ADD CONSTRAINT fk_media_owner FOREIGN KEY (owner_user_id) REFERENCES public.users(user_id) ON DELETE CASCADE;


--
-- Name: messages fk_messages_media; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT fk_messages_media FOREIGN KEY (media_id) REFERENCES public.media(media_id) ON DELETE SET NULL;


--
-- Name: messages fk_messages_reply; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT fk_messages_reply FOREIGN KEY (reply_to_message_id) REFERENCES public.messages(message_id) ON DELETE SET NULL;


--
-- Name: users fk_users_profile_image; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT fk_users_profile_image FOREIGN KEY (profile_image_media_id) REFERENCES public.media(media_id) ON DELETE SET NULL;


--
-- Name: message_reactions message_reactions_message_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.message_reactions
    ADD CONSTRAINT message_reactions_message_fkey FOREIGN KEY (message_id) REFERENCES public.messages(message_id) ON DELETE CASCADE;


--
-- Name: message_reactions message_reactions_user_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.message_reactions
    ADD CONSTRAINT message_reactions_user_fkey FOREIGN KEY (user_id) REFERENCES public.users(user_id) ON DELETE CASCADE;


--
-- Name: messages messages_sender_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_sender_id_fkey FOREIGN KEY (sender_id) REFERENCES public.users(user_id);


--
-- Name: private_chats private_chats_chatroom_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_chatroom_fk FOREIGN KEY (chatroom_id) REFERENCES public.chatrooms(id) ON DELETE CASCADE;


--
-- Name: private_chats private_chats_user1_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_user1_fk FOREIGN KEY (user1_id) REFERENCES public.users(user_id) ON DELETE CASCADE;


--
-- Name: private_chats private_chats_user2_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.private_chats
    ADD CONSTRAINT private_chats_user2_fk FOREIGN KEY (user2_id) REFERENCES public.users(user_id) ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

